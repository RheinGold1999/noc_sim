#ifndef __DATA_TYPE_H__
#define __DATA_TYPE_H__

// #include "noc/common.h"

#include <list>
#include <set>
#include <array>
#include <cstdint>
#include <string>

/**
 * @brief NodeAddr is for router decision, which works similarly like IP addresses.
 *        `m_addr[i] = -1` means the i-level address is masked.
 */

class Coord;

class NodeAddr
{
public:
  static const int MAX_LEVEL = 4;
  static const int MASKED = -1;   // used by BridgeRouter

public:
  NodeAddr(int addr3 = 0, int addr2 = 0, int addr1 = 0, int addr0 = 0);

  void set(int lvl, int val);
  int get(int lvl) const;

  /**
   * @brief If any of m_addr[i] or other.m_addr[i] is -1, then i-level is matched.
   *        If all levels are matched, the result is true.
   */
  bool is_matched(const NodeAddr& other) const;
  bool is_matched(const Coord& coord) const;

  void reset();

  std::string to_str() const;

private:
  std::array<int, MAX_LEVEL> m_addr;
};


/**
 * @brief Coord is used to index all Nodes in the network
 * @param m_addr: node addr (e.g. 4.3.2.1)
 * @param m_id: unique index of Node in the network, corresponds to a certain node addr
 */
class Coord
{
public:
  Coord(const NodeAddr& addr);
  Coord(int id);
  Coord();

  void set_id_from_node_addr(const NodeAddr& addr);
  void set_node_addr_from_id(int id);
  void reset();
  bool is_equal(const Coord& other);
  NodeAddr get_addr() const;
  int get_id() const;
  std::string to_str() const;

private:
  NodeAddr m_addr;
  int m_id;
};

class Flit;

class Packet
{
  friend class PacketManager;

public:
  enum class PktType : int
  {
    UNKNOWN,
    READ_REQ,
    READ_RSP,
    WRITE_REQ,
    WRITE_RSP,
  };

  enum class Parity: int
  {
    CW = 0,
    ACW = 1,
    DONT_CARE = -1,
  };

private:
  Coord m_src;
  Coord m_dst;
  uint64_t m_id;
  PktType m_type;

  int m_total_flits_num;
  uint64_t m_creation_time;

  Parity m_parity;
  int m_rsp_flit_num;
  Packet* m_req_pkt;  // for rsp packet only

public:
  int m_arrived_flits_num;
  std::list<Flit*> m_flits_list;
  uint64_t m_injection_time;

private:
  Packet(
    const Coord& src, 
    const Coord& dst, 
    PktType type,
    int flits_num, 
    uint64_t creation_time,
    Parity parity = Parity::DONT_CARE,
    Packet* req_pkt = nullptr
  );

  void init(
    const Coord& src, 
    const Coord& dst, 
    PktType type,
    int flits_num, 
    uint64_t creation_time,
    Parity parity = Parity::DONT_CARE,
    Packet* req_pkt = nullptr
  );

public:
  int get_rsp_flit_num() const;
  void set_rsp_flit_num(int num);

  PktType get_type() const;
  Coord get_src() const;
  Coord get_dst() const;
  Packet* get_req_pkt() const;
  Flit* pop_flit();
  int rest_flit_num() const;
};

class PacketManager
{
public:
  static Packet* acquire(
    const Coord& src,
    const Coord& dst,
    Packet::PktType type,
    int flits_num,
    uint64_t creation_time,
    Packet::Parity parity = Packet::Parity::DONT_CARE,
    Packet* req_pkt = nullptr
  );
  static void release(Packet* pkt);
  static void destory();
  static int alloc_pkt_id();

private:
  static uint64_t s_alloc_pkt_cnt;    // records currently created packets number
  static uint64_t s_newed_pkt_cnt;    // records currently newed packets number
  static std::list<Packet*> s_pool;   // serves as memory manager
  static std::set<Packet*> s_inflights;  // for debug purpose
};

class Flit
{
  friend class Packet;
  friend class FlitManager;

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
  
  Coord get_dst() const;
  Coord get_src() const;

  Packet* get_pkt();

private:
  Flit(Packet* owner, int id);
  void init(Packet* owner, int id);
};

class FlitManager
{
public:
  static Flit* acquire(Packet* owner, int id);
  static void release(Flit* flit);
  static void destroy();             // will be invoked before the end of simulation

private:
  static uint64_t s_newed_flit_cnt;   // records currently newed flits number
  static std::list<Flit*> s_pool;
};


#endif  /* __DATA_TYPE_H__ */
