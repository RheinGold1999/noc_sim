#include "noc/node.h"
#include "noc/double_ring.h"
#include "model_utils/port.h"
#include "model_utils/simulator.h"
#include "config/noc_config.h"
#include "log/logger.h"

Node::Node(const ModelBase* parent, const std::string& name, const Coord& coord) :
  ModuleBase(parent, name),
  m_coord(coord)
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

  m_arb_flits = new Flit* [NocConfig::ring_width];

  INFO("created: {}", m_coord.to_str());
}

Node::~Node()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    delete flit_o[i];
    delete flit_i[i];
  }
  delete [] flit_o;
  delete [] flit_i;
  
  delete [] m_arb_flits;
}

void
Node::transfer()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (flit_i[i]->can_read()) {
      Flit* flit = flit_i[i]->read();
      Packet* pkt = flit->get_pkt();
      DEBUG("rcv flit from node_router: {}", flit->to_str());
      if (flit->is_tail()) {
        rcv_pkt(pkt);
      } else if (flit->is_head()) {
        ASSERT(m_rob_map.count(pkt) == 0);
        m_rob_map[pkt] = {flit};
      } else {
        ASSERT(m_rob_map.count(pkt) == 1);
        m_rob_map[pkt].emplace_back(flit);
      }
    }
  }

  gen_req_pkt();
}

void
Node::process()
{
  inj_arb();
}

void
Node::update()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (m_arb_flits[i] && flit_o[i]->can_write()) {
      flit_o[i]->write(m_arb_flits[i]);
      if (m_arb_flits[i]->is_tail()) {
        m_inflight_req_set.insert(m_arb_flits[i]->get_pkt());
      }
      DEBUG("inj flit: {}", m_arb_flits[i]->to_str());
      m_arb_flits[i] = nullptr;
    }
  }
}

void
Node::gen_req_pkt()
{
  static int cyc_intval = 1.0 * 2 / NocConfig::node_inj_rate;
  if (Simulator::curr_tick() % cyc_intval == 0) {
    Packet* pkt = PacketManager::acquire(
      m_coord,
      DoubleRing::gen_random_dst(m_coord),
      Packet::PktType::WRITE_REQ,
      1,
      Simulator::curr_tick()
    );
    m_req_que.emplace_back(pkt);
    // DEBUG("gen req, req_que.size: {}", m_req_que.size());
  }
}

void
Node::gen_rsp_pkt(Packet* req_pkt)
{
  int rsp_flit_num = req_pkt->get_rsp_flit_num();
  Packet::PktType rsp_type;
  if (req_pkt->get_type() == Packet::PktType::READ_REQ) {
    rsp_type = Packet::PktType::READ_RSP;
  } else if (req_pkt->get_type() == Packet::PktType::WRITE_REQ) {
    rsp_type = Packet::PktType::WRITE_RSP;
  } else {
    ERROR("Wrong req_pkt type: {}", (int)(req_pkt->get_type()));
  }
  Coord rsp_src = req_pkt->get_dst();
  Coord rsp_dst = req_pkt->get_src();
  Packet* rsp_pkt = PacketManager::acquire(
    rsp_src,
    rsp_dst,
    rsp_type,
    rsp_flit_num,
    Simulator::curr_tick(),
    Packet::Parity::DONT_CARE,
    req_pkt
  );
  INFO("rcv req_pkt: {}", req_pkt->to_str());
  INFO("gen rsp_pkt: {}", rsp_pkt->to_str());
  m_rsp_que.emplace_back(rsp_pkt);
}

void
Node::inj_arb()
{
  static uint64_t req_rsp_rr = 0;
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (m_arb_flits[i]) {
      // The output flit_o[i] is block
      continue;
    }

    Flit* flit = nullptr;
    if (req_rsp_rr % 2 == 0) {
      // req first
      flit = get_next_flit(m_req_que);
      if (!flit) {
        flit = get_next_flit(m_rsp_que);
      }
    } else {
      // rsp first
      flit = get_next_flit(m_rsp_que);
      if (!flit) {
        flit = get_next_flit(m_req_que);
      }
    }
    m_arb_flits[i] = flit;
  }
  req_rsp_rr++;
}

void
Node::rcv_pkt(Packet* pkt)
{
  if (m_rob_map.count(pkt)) {
    m_rob_map.erase(pkt);
  }

  switch (pkt->get_type()) {
    case (Packet::PktType::READ_REQ):
    case (Packet::PktType::WRITE_REQ): {
      gen_rsp_pkt(pkt);
      break;
    }
    case (Packet::PktType::READ_RSP):
    case (Packet::PktType::WRITE_RSP): {
      Packet* req_pkt = pkt->get_req_pkt();
      INFO("rcv rsp: {}", pkt->to_str());
      INFO("cor req: {}", req_pkt->to_str());
      ASSERT(m_inflight_req_set.count(req_pkt) == 1);
      m_inflight_req_set.erase(req_pkt);
      // TODO: statistics for req_pkt

      PacketManager::release(pkt);
      PacketManager::release(req_pkt);
      break;
    }
    default: {
      ERROR("unknown pkt type: {}", pkt->to_str());
      break;
    }
  }
}

Flit*
Node::get_next_flit(std::list<Packet*>& pkt_que)
{
  Flit* flit = nullptr;
  if (!pkt_que.empty()) {
    Packet* pkt = pkt_que.front();
    flit = pkt->pop_flit();
    if (pkt->rest_flit_num() == 0) {
      pkt_que.pop_front();
    }
  }
  return flit;
}

