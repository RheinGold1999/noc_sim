#include "noc/bridge_router.h"
#include "model_utils/port.h"
#include "model_utils/fifo.h"
#include "config/noc_config.h"

BridgeRouter::BridgeRouter(
  const ModelBase* parent,
  const std::string& name,
  const Coord& coord
) :
  Router(parent, name),
  m_coord(coord)
{
  ASSERT(NocConfig::ring_width % 2 == 0);

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
    os.str("");
    os << "loc_inj_o_" << i;
    loc_inj_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.str("");
    os << "loc_eje_i_" << i;
    loc_eje_i[i] = new StreamPortIn<Flit*>(this, os.str());

    os.str("");
    os << "glb_inj_o_" << i;
    glb_inj_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.str("");
    os << "glb_eje_i_" << i;
    glb_eje_i[i] = new StreamPortIn<Flit*>(this, os.str());

    os.str("");
    os << "loc2glb_que_" << i;
    m_loc2glb_que[i] = new FIFO<Flit*>(this, os.str(), NocConfig::bridge_inj_que_depth);

    os.str("");
    os << "glb2loc_que_" << i;
    m_glb2loc_que[i] = new FIFO<Flit*>(this, os.str(), NocConfig::bridge_inj_que_depth);

    m_loc_arb_flits[i] = nullptr;
    m_glb_arb_flits[i] = nullptr;
  }
  INFO("created: {}", m_coord.to_str());
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
  // Swap rule
  // ---------------------------------------------------------------------------
  bool swaped[NocConfig::ring_width.get_val()];
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    Flit* loc_flit = 
      loc_eje_i[i]->can_read() ? loc_eje_i[i]->read() : nullptr;
    Flit* glb_flit = 
      glb_eje_i[i]->can_read() ? glb_eje_i[i]->read() : nullptr;
    
    if (loc_flit && glb_flit && is_loc2glb(loc_flit) && is_glb2loc(glb_flit)) {
      m_loc_arb_flits[i] = glb_flit;
      m_glb_arb_flits[i] = loc_flit;
      swaped[i] = true;
    } else {
      swaped[i] = false;
    }
  }

  // ---------------------------------------------------------------------------
  // Local to Global
  // ---------------------------------------------------------------------------
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (swaped[i]) {
      continue;
    }
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
    if (swaped[i]) {
      continue;
    }
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
}

void
BridgeRouter::process()
{
  // ---------------------------------------------------------------------------
  // Strategy 1: Direct Link
  //   - m_glb2loc_que[i] <-> loc_inj_o[i]
  //   - m_loc2glb_que[i] <-> glb_inj_o[i]
  // ---------------------------------------------------------------------------
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (m_loc_arb_flits[i] == nullptr && m_glb2loc_que[i]->can_read()) {
      m_loc_arb_flits[i] = m_glb2loc_que[i]->read();
    }
    if (m_glb_arb_flits[i] == nullptr && m_loc2glb_que[i]->can_read()) {
      m_glb_arb_flits[i] = m_loc2glb_que[i]->read();
    }
  }

  // TODO: try other strategies
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
      ERROR("bridge addr {} is unexpected!!!", addr.to_str());
      abort();
    }
    if (val != NodeAddr::MASKED) {
      found_non_mask = true;
    }
  }
}

NodeAddr
BridgeRouter::get_addr() const
{
  return m_addr;
}

void
BridgeRouter::set_addr(const NodeAddr& addr)
{
  check_addr(addr);
  m_addr = addr;
  INFO("set addr to {}", m_addr.to_str());
}
