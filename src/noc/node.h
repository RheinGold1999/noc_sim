#ifndef __TRACE_SENDER_H__
#define __TRACE_SENDER_H__

#include <list>
#include <map>

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
  Node(const ModelBase* parent, const std::string& name);
  ~Node();

  void transfer() override;
  void process() override;
  void update() override;

private:
  void gen_req_pkt();
  void gen_rsp_pkt();

private:
  std::list<Packet*> m_issue_que;     // packets waiting to be issued to NodeRouter
  // std::list<Packet*> m_inflight_que;  // packets been issed and waiting for response
  std::map<Packet*, std::list<Flit*>>  m_inflight_map;  // also as ROB
};

#endif /* __TRACE_SENDER_H__ */
