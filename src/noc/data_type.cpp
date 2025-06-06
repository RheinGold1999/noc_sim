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
  os << "{id: " << m_id;
  os << ", addr: " << m_addr.to_str();
  os << "}";
  return os.str();
}

void
Coord::set_addr(int lvl, int val)
{
  m_addr.set(lvl, val);
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
  , m_poped_flits_num(0)
  , m_arrived_flits_num(0)
  , m_parity(parity)
  , m_creation_time(creation_time)
  , m_injection_time(UINT64_MAX)
  , m_rsp_flit_num(1)
  , m_req_pkt(req_pkt)
{
  m_flits_vec.clear();
  for (int i = 0; i < m_total_flits_num; ++i) {
    m_flits_vec.emplace_back(FlitManager::acquire(this, i));
  }
  m_flits_vec.front()->m_is_head = true;
  m_flits_vec.back()->m_is_tail = true;
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
  m_poped_flits_num = 0;
  m_arrived_flits_num = 0;
  m_parity = parity;

  m_creation_time = creation_time;
  m_injection_time = UINT16_MAX;

  m_rsp_flit_num = 1;
  m_req_pkt = req_pkt;

  _ASSERT(m_flits_vec.empty());
  for (int i = 0; i < m_total_flits_num; ++i) {
    m_flits_vec.emplace_back(FlitManager::acquire(this, i));
  }
  m_flits_vec.front()->m_is_head = true;
  m_flits_vec.back()->m_is_tail = true;
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

Flit* 
Packet::pop_flit()
{
  Flit* flit = nullptr;
  // if (!m_flits_list.empty()) {
  //   flit = m_flits_list.front();
  //   m_flits_list.pop_front();
  // }
  if (m_poped_flits_num < m_total_flits_num) {
    flit = m_flits_vec[m_poped_flits_num];
    m_poped_flits_num++;
  }
  return flit; 
}

int
Packet::rest_flit_num() const
{
  return m_total_flits_num - m_poped_flits_num;
}

bool
Packet::is_req() const
{
  return m_type == PktType::READ_REQ || m_type == PktType::WRITE_REQ;
}

bool
Packet::is_rsp() const
{
  return m_type == PktType::READ_RSP || m_type == PktType::WRITE_RSP;
}

std::string
Packet::to_str() const
{
  static std::string pkt_type_str[] = {
    "UNKNOWN", "READ_REQ", "READ_RSP", "WRITE_REQ", "WRITE_RSP"
  };
  std::ostringstream os;
  os << "{" 
    << "id: " << m_id
    << ", src: " << m_src.get_addr().to_str()
    << ", dst: " << m_dst.get_addr().to_str()
    << ", type: " << pkt_type_str[(int)m_type]
    << ", creation_time: " << m_creation_time
    << ", flit_num: " << m_total_flits_num
    << ", time_passed: " << (Simulator::curr_tick() - m_creation_time)
    << "}";

  return os.str();
}

// -----------------------------------------------------------------------------
// Packet Manager
// -----------------------------------------------------------------------------
uint64_t PacketManager::s_alloc_pkt_cnt = 0;
uint64_t PacketManager::s_newed_pkt_cnt = 0;
std::list<Packet*> PacketManager::s_pool;
std::set<Packet*> PacketManager::s_inflights;

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
  _ASSERT(s_inflights.find(pkt) == s_inflights.end());
  s_inflights.insert(pkt);
  return pkt;
}

void
PacketManager::release(Packet* pkt)
{
  for (auto flit : pkt->m_flits_vec) {
    FlitManager::release(flit);
  }
  pkt->m_flits_vec.clear();

  _ASSERT(s_inflights.find(pkt) != s_inflights.end());
  s_inflights.erase(pkt);

  s_pool.emplace_back(pkt);
}

void
PacketManager::destory()
{
  _INFO("PacketManager: s_inflits.size: {}, s_pool.size: {}, s_newed_pkt_cnt: {}",
    s_inflights.size(), s_pool.size(), s_newed_pkt_cnt);
  if (s_inflights.size() > 0) {
    for (auto pkt : s_inflights) {
      _WARN("inflight packet: {}", pkt->to_str());
      /** ATTENTION: 
       * release(pkt) will change the size of s_inflights, 
       * which is NOT allowed in the for-loop.
       */
      // release(pkt);  // DONT do this
      for (auto flit : pkt->m_flits_vec) {
        _WARN("inflight flit: {}", flit->to_str());
        FlitManager::release(flit);
      }
      pkt->m_flits_vec.clear();
      s_pool.emplace_back(pkt);
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

bool
Flit::is_head() const
{
  return m_is_head;
}

bool
Flit::is_tail() const
{
  return m_is_tail;
}

std::string
Flit::to_str()
{
  std::ostringstream os;
  os << "{"
    <<"id: " << m_owner->m_id << ":" << m_id << "/" << (m_owner->m_total_flits_num - 1)
    << ", src: " << get_src().get_id()
    << ", dst: " << get_dst().get_id()
    << "}";
  return os.str();
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
  _ASSERT(
    std::find(s_pool.begin(), s_pool.end(), flit) == s_pool.end()
  );
  s_pool.emplace_back(flit);
}

void
FlitManager::destroy()
{
  _INFO("FlitManager: s_pool.size: {}, s_newed_flit_cnt: {}",
    s_pool.size(), s_newed_flit_cnt);
  _ASSERT(s_pool.size() == s_newed_flit_cnt);
  for (auto flit : s_pool) {
    delete flit;
  }
}

