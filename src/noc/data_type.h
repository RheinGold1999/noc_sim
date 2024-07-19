#ifndef __DATA_TYPE_H__
#define __DATA_TYPE_H__

#include "noc_config.h"
#include "global_config.h"

#include <list>
#include <set>
#include <stdint.h>

/**
 * @brief Coord is used to index all Nodes in the network
 * @param m_x: x coordinate
 * @param m_y: y coordinate
 * @param m_id: unique index of Node in the network, corresponds to a certain (x, y)
 * 
 * @example:
 *   y ^
 *     |
 *     *3   *7   *11  *15
 *     |
 *     *2   *6   *10  *14
 *     |
 *     *1   *5   *9   *13
 *     |    
 *     *0 --*4 --*8 --*12 ---->
 *                           x
 */
class Coord
{
public:
  int m_x;
  int m_y;
  int m_id;

public:
  Coord(int x, int y);
  Coord(int id);
  Coord();

  void setIDfromXY(int x, int y);
  void setXYfromID(int id);

  void reset();
};


class Packet
{
public:
  Coord m_src;
  Coord m_dst;
  uint64_t m_id;

  int m_total_flits_num;
  int m_arrived_flits_num;
  std::list<Flit*> m_flits_list;

  uint64_t m_creation_time;
  uint64_t m_injection_time;

  NocConfig::parity_e m_parity;

private:
  Packet(
    const Coord& src, 
    const Coord& dst, 
    int flits_num, 
    uint64_t creation_time,
    NocConfig::parity_e parity = NocConfig::PARITY_DONT_CARE
  );

  void init(
    const Coord& src, 
    const Coord& dst, 
    int flits_num, 
    uint64_t creation_time,
    NocConfig::parity_e parity = NocConfig::PARITY_DONT_CARE
  );

  // ---------------------------------------------------------------------------
  // static members and methods
  // ---------------------------------------------------------------------------
private:
  static uint64_t s_created_pkt_cnt;  // records currently created packets number
  static uint64_t s_newed_pkt_cnt;    // records currently newed packets number
  static std::list<Packet*> s_pool;   // serves as memory manager
  static std::set<Packet*> s_inflights;  // for debug purpose

public:
  static Packet* acquire(
    const Coord& src,
    const Coord& dst,
    int flits_num,
    uint64_t creation_time,
    NocConfig::parity_e parity = NocConfig::PARITY_DONT_CARE
  );                                  // serves as memory manager
  static void release(Packet* pkt);   // serves as memory manager
  static void destory();              // will be invoked before the end of simulation
};


class Flit
{
  friend class Packet;
public:
  Packet* m_owner;
  int m_id;
  bool m_is_head;
  bool m_is_tail;

  bool m_is_deflected;
  uint64_t m_deflections_cnt;

  Coord m_curr_coord;
  // Coord m_next_coord;

  uint64_t m_time_into_buf;
  uint64_t m_time_spent_in_buf;
  uint64_t m_time_wait_to_inj;
  uint64_t m_time_in_src_ring;
  uint64_t m_time_in_dst_ring;
  uint64_t m_time_in_imd_ring;  // intermediate ring

private:
  Flit(Packet* owner, int id);
  void init(Packet* owner, int id);

  // ---------------------------------------------------------------------------
  // static members and methods (serves as memory manager)
  // ---------------------------------------------------------------------------
private:
  static uint64_t s_newed_flit_cnt;   // records currently newed flits number
  static std::list<Flit*> s_pool;
  static Flit* acquire(Packet* owner, int id);
  static void release(Flit* flit);
  static void destroy();             // will be invoked before the end of simulation
};


#endif  /* __DATA_TYPE_H__ */
