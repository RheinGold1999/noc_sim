#ifndef __TRACE_SENDER_H__
#define __TRACE_SENDER_H__

#include <list>
#include <map>
#include <set>

#include "model_utils/module_base.h"
// #include "model_utils/port.h"
#include "noc/data_type.h"

template <typename T>
class StreamPortOut;

template <typename T>
class StreamPortIn;

class Node :
  public ModuleBase
{
public:
  StreamPortOut<Flit*>** flit_o;
  StreamPortIn<Flit*>** flit_i;

public:
  Node(const ModelBase* parent, const std::string& name, const Coord& coord);
  ~Node();

  void transfer() override;
  void process() override;
  void update() override;

private:
  void gen_req_pkt();
  void gen_rsp_pkt(Packet* req_pkt);
  void inj_arb();
  void rcv_pkt(Packet* pkt);
  Flit* get_next_flit(std::list<Packet*>& pkt_que);

private:
  Coord m_coord;
  std::list<Packet*> m_req_que;     // packets waiting to be issued to NodeRouter
  std::list<Packet*> m_rsp_que;
  Flit** m_arb_flits;
  std::map<Packet*, std::list<Flit*>>  m_rob_map;  // can also be seen as a ROB
  std::set<Packet*> m_inflight_req_set;
};

#endif /* __TRACE_SENDER_H__ */
