#include "noc/node.h"
#include "model_utils/port.h"
#include "model_utils/simulator.h"
#include "config/noc_config.h"
#include "log/logger.h"

Node::Node(const ModelBase* parent, const std::string& name) :
  ModuleBase(parent, name)
{
  m_req_que.clear();
  m_rsp_que.clear();
  m_rob_map.clear();
  m_inflight_req_set.clear();

  flit_o = new StreamPortOut<Flit*>* [NocConfig::ring_width];
  flit_i = new StreamPortIn<Flit*>* [NocConfig::ring_width];

  std::ostringstream os;
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    os.str("");
    os << "flit_o_" << i;
    flit_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.str("");
    os << "flit_i_" << i;
    flit_i[i] = new StreamPortIn<Flit*>(this, os.str());
  }

  INFO("created");
}

Node::~Node()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    delete flit_o[i];
    delete flit_i[i];
  }
  delete [] flit_o;
  delete [] flit_i;
}

void
Node::transfer()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (flit_i[i]->can_read()) {
      Flit* flit = flit_i[i]->read();
      Packet* pkt = flit->get_pkt();
      if (flit->m_is_head) {
        ASSERT(m_rob_map.count(pkt) == 0);
        m_rob_map[pkt] = {flit};
      } else if (flit->m_is_tail) {
        ASSERT(m_rob_map.count(pkt) == 1);
        rcv_pkt(pkt);
      } else {
        ASSERT(m_rob_map.count(pkt) == 1);
        m_rob_map[pkt].emplace_back(flit);
      }
    }
  }
}

void
Node::process()
{

}

void
Node::update()
{

}

void
Node::gen_req_pkt()
{

}

void
Node::gen_rsp_pkt()
{
  
}

void
Node::rcv_pkt(Packet* pkt)
{
  switch (pkt->get_type()) {
    case (Packet::PktType::READ_REQ): {
      int rsp_flit_num = pkt->get_rsp_flit_num();
      Packet* rsp_pkt = PacketManager::acquire(
        pkt->get_dst(),
        pkt->get_src(),
        Packet::PktType::READ_RSP,
        rsp_flit_num,
        Simulator::curr_tick(),
        Packet::Parity::DONT_CARE,
        pkt
      );
      m_rsp_que.emplace_back(rsp_pkt);
      break;
    }

    case (Packet::PktType::WRITE_REQ): {
      Packet* rsp_pkt = PacketManager::acquire(
        pkt->get_dst(),
        pkt->get_src(),
        Packet::PktType::WRITE_RSP,
        1,
        Simulator::curr_tick(),
        Packet::Parity::DONT_CARE,
        pkt
      );     
      m_rsp_que.emplace_back(rsp_pkt);
      break;
    }

    case (Packet::PktType::READ_RSP):
    case (Packet::PktType::WRITE_RSP): {
      Packet* req_pkt = pkt->get_req_pkt();
      ASSERT(m_inflight_req_set.count(req_pkt) == 1);
      m_inflight_req_set.erase(req_pkt);
      // TODO: statistics for req_pkt
    }

    default: {
      ERROR("unknown Packet Type");
      break;
    }
  }

  m_rob_map.erase(pkt);
  PacketManager::release(pkt);
}
