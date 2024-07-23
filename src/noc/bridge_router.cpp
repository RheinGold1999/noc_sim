#include "noc/bridge_router.h"
#include "model_utils/port.h"
#include "model_utils/fifo.h"
#include "config/noc_config.h"

BridgeRouter::BridgeRouter(
  const ModelBase* parent,
  const std::string& name,
  const NodeAddr& addr
)
  : ModuleBase(parent, name),
    m_addr(addr)
{
  check_addr(addr);

  loc_inj_o = new StreamPortOut<Flit*>* [NocConfig::ring_width];
  loc_eje_i = new StreamPortIn<Flit*>* [NocConfig::ring_width];
  glb_inj_o = new StreamPortOut<Flit*>* [NocConfig::ring_width];
  glb_eje_i = new StreamPortIn<Flit*>* [NocConfig::ring_width];

  m_loc2glb_que = new FIFO<Flit*>* [NocConfig::ring_width];
  m_glb2loc_que = new FIFO<Flit*>* [NocConfig::ring_width];

  m_loc_arb_flits = new Flit* [NocConfig::ring_width];
  m_glb_arb_flits = new Flit* [NocConfig::ring_width];

  std::ostringstream os;
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    os.clear();
    os << "loc_inj_o_" << i;
    loc_inj_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.clear();
    os << "loc_eje_i_" << i;
    loc_eje_i[i] = new StreamPortIn<Flit*>(this, os.str());

    os.clear();
    os << "glb_inj_o_" << i;
    glb_inj_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.clear();
    os << "glb_eje_i_" << i;
    glb_eje_i[i] = new StreamPortIn<Flit*>(this, os.str());

    os.clear();
    os << "loc2glb_que_" << i;
    m_loc2glb_que[i] = new FIFO<Flit*>(this, os.str(), NocConfig::bridge_inj_que_depth);

    os.clear();
    os << "glb2loc_que_" << i;
    m_glb2loc_que[i] = new FIFO<Flit*>(this, os.str(), NocConfig::bridge_inj_que_depth);

    m_loc_arb_flits[i] = nullptr;
    m_glb_arb_flits[i] = nullptr;
  }
}

BridgeRouter::~BridgeRouter()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    delete loc_inj_o[i];
    delete loc_eje_i[i];
    delete glb_inj_o[i];
    delete glb_eje_i[i];
    delete m_loc2glb_que[i];
    delete m_glb2loc_que[i];
  }

  delete [] loc_inj_o;
  delete [] loc_eje_i;
  delete [] glb_inj_o;
  delete [] glb_eje_i;
  delete [] m_loc2glb_que;
  delete [] m_glb2loc_que;

  delete [] m_loc_arb_flits;
  delete [] m_glb_arb_flits;
}

void
BridgeRouter::transfer()
{
  // ---------------------------------------------------------------------------
  // Local to Global
  // ---------------------------------------------------------------------------
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (loc_eje_i[i]->can_read()) {
      Flit* flit = loc_eje_i[i]->read();
      if (is_loc2glb(flit)) {
        if (m_loc2glb_que[i]->can_write()) {
          m_loc2glb_que[i]->write(flit);
        } else {
          flit->m_is_deflected = true;
          flit->m_deflections_cnt++;
          m_loc_arb_flits[i] = flit;
        }
      } else {
        m_loc_arb_flits[i] = flit;
      }
    }
  }

  // ---------------------------------------------------------------------------
  // Global to Local
  // ---------------------------------------------------------------------------
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (glb_eje_i[i]->can_read()) {
      Flit* flit = glb_eje_i[i]->read();
      if (is_glb2loc(flit)) {
        if (m_glb2loc_que[i]->can_write()) {
          m_glb2loc_que[i]->write(flit);
        } else {
          flit->m_is_deflected = true;
          flit->m_deflections_cnt++;
          m_glb_arb_flits[i] = flit;
        }
      } else {
        m_glb_arb_flits[i] = flit;
      }
    }
  }

  // TODO: add `Swap Rule`
}

void
BridgeRouter::process()
{
  // TODO: implement the crossbar logic here

}

void
BridgeRouter::update()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    assert(loc_inj_o[i]->can_write());
    if (m_loc_arb_flits[i]) {
      loc_inj_o[i]->write(m_loc_arb_flits[i]);
      m_loc_arb_flits[i] = nullptr;
    }

    assert(glb_inj_o[i]->can_write());
    if (m_glb_arb_flits[i]) {
      glb_inj_o[i]->write(m_glb_arb_flits[i]);
      m_glb_arb_flits[i] = nullptr;
    }
  }
}

bool
BridgeRouter::is_loc2glb(const Flit* flit)
{
  return !m_addr.is_matched(flit->get_dst());
}

bool
BridgeRouter::is_glb2loc(const Flit* flit)
{
  return m_addr.is_matched(flit->get_dst());
}

void
BridgeRouter::check_addr(const NodeAddr& addr)
{
  bool found_non_mask = false;
  for (int i = 0; i < NodeAddr::MAX_LEVEL; ++i) {
    int val = addr.get(i);
    if (found_non_mask && val == NodeAddr::MASKED) {
      ERROR("bridge addr {}.{}.{}.{} is unexpected!!!", 
        addr.get(3), addr.get(2), addr.get(1), addr.get(0));
      abort();
    }
    if (val != NodeAddr::MASKED) {
      found_non_mask = true;
    }
  }
}
