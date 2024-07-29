#include "noc/data_type.h"
#include "config/noc_config.h"
#include "log/logger.h"

#include <sstream>

// -----------------------------------------------------------------------------
// NodeAddr
// -----------------------------------------------------------------------------

NodeAddr::NodeAddr(int addr3, int addr2, int addr1, int addr0)
  : m_addr({addr0, addr1, addr2, addr3})
{}

void
NodeAddr::set(int lvl, int addr)
{
  _ASSERT(lvl < MAX_LEVEL);
  m_addr[lvl] = addr;
}

int
NodeAddr::get(int lvl) const
{
  return m_addr[lvl];
}

bool
NodeAddr::is_matched(const NodeAddr& other) const
{
  bool match = true;
  for (int i = 0; i < MAX_LEVEL; ++i) {
    if ((m_addr[i] == MASKED) || (other.m_addr[i] == MASKED)) {
      continue;
    }
    if (m_addr[i] != other.m_addr[i]) {
      match = false;
      break;
    }
  }
  return match;
}

bool
NodeAddr::is_matched(const Coord& coord) const
{
  return is_matched(coord.get_addr());
}

void
NodeAddr::reset()
{
  for (int i = 0; i < MAX_LEVEL; ++i) {
    m_addr[i] = 0;
  }
}

std::string
NodeAddr::to_str() const
{
  std::ostringstream os;
  os << "NodeAddr: ";
  for (int i = MAX_LEVEL - 1; i >= 0; --i) {
    if (get(i) == MASKED) {
      os << "x";
    } else {
      os << get(i);
    }
    if (i != 0) {
      os << ".";
    }
  }
  return os.str();
}

// -----------------------------------------------------------------------------
// Coord
// -----------------------------------------------------------------------------

Coord::Coord(const NodeAddr& addr)
  : m_addr(addr)
{
  set_id_from_node_addr(addr);
}

Coord::Coord(int id)
  : m_id(id)
{
  set_node_addr_from_id(id);
}

Coord::Coord()
{
  reset();
}

void
Coord::set_id_from_node_addr(const NodeAddr& addr)
{
  static const int id_0_weight = 1;

  static const int id_1_weight = 
    NocConfig::network_lvl_0_num;
  
  static const int id_2_weight = 
    NocConfig::network_lvl_0_num * 
    NocConfig::network_lvl_1_num;

  static const int id_3_weight = 
    NocConfig::network_lvl_0_num * 
    NocConfig::network_lvl_1_num * 
    NocConfig::network_lvl_2_num;

  int id = 0;
  id += addr.get(0) * id_0_weight;
  id += addr.get(1) * id_1_weight;
  id += addr.get(2) * id_2_weight;
  id += addr.get(3) * id_3_weight;
  m_id = id;
}

void
Coord::set_node_addr_from_id(int id)
{
  static const int id_0_weight = 1;

  static const int id_1_weight = 
    NocConfig::network_lvl_0_num;

  static const int id_2_weight = 
    NocConfig::network_lvl_0_num * 
    NocConfig::network_lvl_1_num;

  static const int id_3_weight = 
    NocConfig::network_lvl_0_num * 
    NocConfig::network_lvl_1_num * 
    NocConfig::network_lvl_2_num;

  int id_3 = id / id_3_weight;
  m_addr.set(3, id_3);

  id = id % id_3_weight;
  int id_2 = id / id_2_weight;
  m_addr.set(2, id_2);

  id = id % id_2_weight;
  int id_1 = id / id_1_weight;
  m_addr.set(1, id_1);

  id = id % id_1_weight;
  m_addr.set(0, id);
}

void
Coord::reset()
{
  m_addr.reset();
  m_id = -1;  // -1 is invalid for m_id
}

bool
Coord::is_equal(const Coord& other)
{
  return m_addr.is_matched(other.m_addr);
}

NodeAddr
Coord::get_addr() const
{
  return m_addr;
}

int
Coord::get_id() const
{
  return m_id;
}

std::string
Coord::to_str() const
{
  std::ostringstream os;
  os << "Coord: {id: " << m_id;
  os << " addr: " << m_addr.to_str();
  os << "}";
  return os.str();
}

// -----------------------------------------------------------------------------
// Packet
// -----------------------------------------------------------------------------

Packet::Packet(
  const Coord& src, 
  const Coord& dst,
  PktType type, 
  int flits_num,
  uint64_t creation_time,
  Parity parity,
  Packet* req_pkt
) 
  : m_src(src)
  , m_dst(dst)
  , m_id(PacketManager::alloc_pkt_id())
  , m_type(type)
  , m_total_flits_num(flits_num)
  , m_arrived_flits_num(0)
  , m_parity(parity)
  , m_creation_time(creation_time)
  , m_injection_time(UINT64_MAX)
  , m_rsp_flit_num(1)
  , m_req_pkt(req_pkt)
{
  m_flits_list.clear();
  for (int i = 0; i < m_total_flits_num; ++i) {
    m_flits_list.push_back(FlitManager::acquire(this, i));
  }
  m_flits_list.front()->m_is_head = true;
  m_flits_list.back()->m_is_tail = true;
}

void
Packet::init(
  const Coord& src, 
  const Coord& dst, 
  PktType type,
  int flits_num, 
  uint64_t creation_time,
  Parity parity,
  Packet* req_pkt
)
{
  m_src = src;
  m_dst = dst;
  m_id = PacketManager::alloc_pkt_id();
  m_type = type;

  m_total_flits_num = flits_num;
  m_arrived_flits_num = 0;
  m_parity = parity;

  m_creation_time = creation_time;
  m_injection_time = UINT16_MAX;

  m_rsp_flit_num = 1;
  m_req_pkt = req_pkt;

  _ASSERT(m_flits_list.empty());
  for (int i = 0; i < m_total_flits_num; ++i) {
    m_flits_list.push_back(FlitManager::acquire(this, i));
  }
  m_flits_list.front()->m_is_head = true;
  m_flits_list.back()->m_is_tail = true;
}

int
Packet::get_rsp_flit_num() const
{
  return m_rsp_flit_num;
}

void
Packet::set_rsp_flit_num(int num)
{
  m_rsp_flit_num = num;
}

Packet::PktType
Packet::get_type() const
{
  return m_type;
}

Coord
Packet::get_src() const
{
  return m_src;
}

Coord
Packet::get_dst() const
{
  return m_dst;
}

Packet*
Packet::get_req_pkt() const
{
  return m_req_pkt;
}

// -----------------------------------------------------------------------------
// Packet Manager
// -----------------------------------------------------------------------------
uint64_t PacketManager::s_alloc_pkt_cnt = 0;
uint64_t PacketManager::s_newed_pkt_cnt = 0;
std::list<Packet*> PacketManager::s_pool = std::list<Packet*>();
std::set<Packet*> PacketManager::s_inflights = std::set<Packet*>();

int
PacketManager::alloc_pkt_id()
{
  return s_alloc_pkt_cnt++;
}

Packet*
PacketManager::acquire(
  const Coord& src,
  const Coord& dst,
  Packet::PktType type,
  int flits_num,
  uint64_t creation_time,
  Packet::Parity parity,
  Packet* req_pkt
)
{
  Packet* pkt = nullptr;
  if (s_pool.empty()) {
    pkt = new Packet(src, dst, type, flits_num, creation_time, parity, req_pkt);
    s_newed_pkt_cnt++;
  } else {
    pkt = s_pool.front();
    s_pool.pop_front();
    pkt->init(src, dst, type, flits_num, creation_time, parity, req_pkt);
  }
  s_inflights.insert(pkt);
  return pkt;
}

void
PacketManager::release(Packet* pkt)
{
  for (auto flit : pkt->m_flits_list) {
    FlitManager::release(flit);
  }
  pkt->m_flits_list.clear();

  _ASSERT(s_inflights.count(pkt) > 0);
  s_inflights.erase(pkt);

  s_pool.push_back(pkt);
}

void
PacketManager::destory()
{
  if (s_inflights.size() > 0) {
    // TODO: print warning and dump all inflight pkt

    for (auto pkt : s_inflights) {
      release(pkt);
    }
  }

  _ASSERT(s_pool.size() == s_newed_pkt_cnt);
  for (auto pkt : s_pool) {
    delete pkt;
  }
}


// -----------------------------------------------------------------------------
// Flit
// -----------------------------------------------------------------------------

Flit::Flit(Packet* owner, int id)
  : m_owner(owner)
  , m_id(id)
  , m_is_head(false)
  , m_is_tail(false)
  , m_is_deflected(false)
  , m_deflections_cnt(0)
  , m_curr_coord()
  // , m_next_coord()
  , m_time_into_buf(0)
  , m_time_spent_in_buf(0)
  , m_time_wait_to_inj(0)
  , m_time_in_src_ring(0)
  , m_time_in_dst_ring(0)
  , m_time_in_imd_ring(0)
{

}

void
Flit::init(Packet* owner, int id)
{
  m_owner = owner;
  m_id = id;
  m_is_head = false;
  m_is_tail = false;

  m_is_deflected = false;
  m_deflections_cnt = 0;

  m_curr_coord.reset();
  // m_next_coord.reset();

  m_time_into_buf = 0;
  m_time_spent_in_buf = 0;
  m_time_wait_to_inj = 0;
  m_time_in_src_ring = 0;
  m_time_in_dst_ring = 0;
  m_time_in_imd_ring = 0;
}

Coord
Flit::get_dst() const
{
  return m_owner->get_dst();
}

Coord
Flit::get_src() const
{
  return m_owner->get_src();
}

Packet*
Flit::get_pkt()
{
  return m_owner;
}

// -----------------------------------------------------------------------------
// Flit Manager
// -----------------------------------------------------------------------------
uint64_t FlitManager::s_newed_flit_cnt = 0;
std::list<Flit*> FlitManager::s_pool = std::list<Flit*>();

Flit*
FlitManager::acquire(Packet* owner, int id)
{
  Flit* flit = nullptr;
  if (s_pool.empty()) {
    flit = new Flit(owner, id);
    s_newed_flit_cnt++;
  } else {
    flit = s_pool.front();
    s_pool.pop_front();
    flit->init(owner, id);
  }
  return flit;
}

void
FlitManager::release(Flit* flit)
{
  s_pool.push_back(flit);
}

void
FlitManager::destroy()
{
  _ASSERT(s_pool.size() == s_newed_flit_cnt);
  for (auto flit : s_pool) {
    delete flit;
  }
}

