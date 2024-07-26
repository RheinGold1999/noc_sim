#include "noc/node_router.h"
#include "model_utils/port.h"
#include "model_utils/fifo.h"
#include "config/noc_config.h"

NodeRouter::NodeRouter(
  const ModelBase* parent, 
  const std::string& name,
  const Coord& coord
)
  : ModuleBase(parent, name),
    m_coord(coord)
{
  inj_o = new StreamPortOut<Flit*>* [NocConfig::ring_width];
  eje_i = new StreamPortIn<Flit*>* [NocConfig::ring_width];

  m_inj_que = new FIFO<Flit*>* [NocConfig::ring_width];
  m_eje_rob = new FIFO<Flit*>* [NocConfig::ring_width];

  m_arb_flits = new Flit* [NocConfig::ring_width];

  std::ostringstream os;
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    os.clear();
    os << "inj_o_" << i;
    inj_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.clear();
    os << "eje_i_" << i;
    eje_i[i] = new StreamPortIn<Flit*>(this, os.str());

    os.clear();
    os << "inj_que_" << i;
    m_inj_que[i] = new FIFO<Flit*>(this, os.str(), NocConfig::node_inj_que_depth);

    os.clear();
    os << "eje_que_" << i;
    m_eje_rob[i] = new FIFO<Flit*>(this, os.str(), NocConfig::node_eje_que_depth);

    m_arb_flits[i] = nullptr;
  }

  m_inflight_pkts.clear();

  INFO("NodeRouter {} is created: {}.", base_name(), m_coord.to_str());
}

NodeRouter::~NodeRouter()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    delete inj_o[i];
    delete eje_i[i];
    delete m_inj_que[i];
    delete m_eje_rob[i];
  }

  delete [] inj_o;
  delete [] eje_i;
  delete [] m_inj_que;
  delete [] m_eje_rob;

  delete [] m_arb_flits;
}

bool
NodeRouter::is_this_dst(const Flit* flit) const
{
  Coord dst = flit->get_dst();
  if (dst.is_equal(this->m_coord)) {
    return true;  
  }
  return false;
}

void
NodeRouter::transfer()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (eje_i[i]->can_read()) {
      Flit* flit = eje_i[i]->read();
      if (is_this_dst(flit)) {
        if (m_eje_rob[i]->can_write()) {
          m_eje_rob[i]->write(flit);
          flit->m_owner->m_arrived_flits_num++;
          if (flit->m_is_tail) {
            receive_pkt(flit->m_owner);
          }
        } else {
          // TODO: write to retransmit buffer

        }
      } else {
        m_arb_flits[i] = flit;
      }
    }
  }
}

void
NodeRouter::process()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (!m_arb_flits[i] && m_inj_que[i]->can_read()) {
      Flit* flit = m_inj_que[i]->read();
      if (flit->m_is_head) {
        m_inflight_pkts.insert(flit->m_owner);
      }
      m_arb_flits[i] = flit;
    }
  }
}

void
NodeRouter::update()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    assert(inj_o[i]->can_write());
    if (m_arb_flits[i]) {
      inj_o[i]->write(m_arb_flits[i]);
      m_arb_flits[i] = nullptr;
    }
  }
}


void
NodeRouter::receive_pkt(Packet* pkt)
{
  // remove all the related flits in ejection robs
  for (auto flit : pkt->m_flits_list) {
    for (int i = 0; i < NocConfig::ring_width; ++i) {
      m_eje_rob[i]->remove(flit);
    }
  }
  m_inflight_pkts.erase(pkt);
  PacketManager::release(pkt);
}

NodeAddr
NodeRouter::get_addr() const
{
  return m_coord.get_addr();
}

