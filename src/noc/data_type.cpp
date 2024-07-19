#include "data_type.h"
#include <assert.h>

// -----------------------------------------------------------------------------
// Coord
// -----------------------------------------------------------------------------

Coord::Coord(int x, int y)
  : m_x(x)
  , m_y(y)
{
  setIDfromXY(x, y);
}

Coord::Coord(int id)
  : m_id(id)
{
  setXYfromID(id);
}

Coord::Coord()
{
  reset();
}

void
Coord::setIDfromXY(int x, int y)
{
  m_id = x * NocConfig::network_nrY + y;
}

void
Coord::setXYfromID(int id)
{
  m_x = id / NocConfig::network_nrY;
  // y = id % NocConfig::network_nrX;
  m_y = id % GlobalConfig::NocConfig::network_nrX;
}

void
Coord::reset()
{
  m_x = -1;
  m_y = -1;
  m_id = -1;
}


// -----------------------------------------------------------------------------
// Packet
// -----------------------------------------------------------------------------

Packet::Packet(
  const Coord& src, 
  const Coord& dst, 
  int flits_num, 
  uint64_t creation_time,
  NocConfig::Parity parity
) 
  : m_src(src)
  , m_dst(dst)
  , m_id(Packet::s_created_pkt_cnt++)
  , m_total_flits_num(flits_num)
  , m_arrived_flits_num(0)
  , m_parity(parity)
  , m_creation_time(creation_time)
  , m_injection_time(UINT64_MAX)
{
  m_flits_list.clear();
  for (int i = 0; i < m_total_flits_num; ++i) {
    m_flits_list.push_back(Flit::acquire(this, i));
  }
  m_flits_list.front()->m_is_head = true;
  m_flits_list.back()->m_is_tail = true;
}

void
Packet::init(
  const Coord& src, 
  const Coord& dst, 
  int flits_num, 
  uint64_t creation_time,
  NocConfig::Parity parity
)
{
  m_src = src;
  m_dst = dst;
  m_id = Packet::s_created_pkt_cnt++;

  m_total_flits_num = flits_num;
  m_arrived_flits_num = 0;
  m_parity = parity;

  m_creation_time = creation_time;
  m_injection_time = UINT16_MAX;

  assert(m_flits_list.empty());
  for (int i = 0; i < m_total_flits_num; ++i) {
    m_flits_list.push_back(Flit::acquire(this, i));
  }
  m_flits_list.front()->m_is_head = true;
  m_flits_list.back()->m_is_tail = true;
}

uint64_t Packet::s_created_pkt_cnt = 0;

uint64_t Packet::s_newed_pkt_cnt = 0;

Packet*
Packet::acquire(
  const Coord& src,
  const Coord& dst,
  int flits_num,
  uint64_t creation_time,
  NocConfig::Parity parity
)
{
  Packet* pkt = nullptr;
  if (s_pool.empty()) {
    pkt = new Packet(src, dst, flits_num, creation_time, parity);
    s_newed_pkt_cnt++;
  } else {
    pkt = s_pool.front();
    s_pool.pop_front();
    pkt->init(src, dst, flits_num, creation_time, parity);
  }
  s_inflights.insert(pkt);
  return pkt;
}

void
Packet::release(Packet* pkt)
{
  for (auto flit : pkt->m_flits_list) {
    Flit::release(flit);
  }
  pkt->m_flits_list.clear();

  assert(s_inflights.count(pkt) > 0);
  s_inflights.erase(pkt);

  s_pool.push_back(pkt);
}

void
Packet::destory()
{
  if (s_inflights.size() > 0) {
    // TODO: print warning and dump all inflight pkt

    for (auto pkt : s_inflights) {
      release(pkt);
    }
  }

  assert(s_pool.size() == s_newed_pkt_cnt);
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

uint64_t Flit::s_newed_flit_cnt = 0;

Flit*
Flit::acquire(Packet* owner, int id)
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
Flit::release(Flit* flit)
{
  s_pool.push_back(flit);
}

void
Flit::destroy()
{
  assert(s_pool.size() == s_newed_flit_cnt);
  for (auto flit : s_pool) {
    delete flit;
  }
}

