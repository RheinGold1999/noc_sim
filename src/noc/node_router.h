#ifndef __NODE_ROUTER_H__
#define __NODE_ROUTER_H__

#include <set>

#include "model_utils/module_base.h"
#include "noc/data_type.h"

template <typename T>
class StreamPortOut;

template <typename T>
class StreamPortIn;

template <typename T>
class FIFO;

class NodeRouter
  : public ModuleBase
{
public:
  StreamPortOut<Flit*>** inj_o;
  StreamPortIn<Flit*>**  eje_i;

public:
  NodeRouter(
    const ModelBase* parent, 
    const std::string& name,
    const NodeAddr& addr
  );
  ~NodeRouter();

  void transfer() override;
  void process() override;
  void update() override;

  bool is_this_dst(const Flit* flit) const;
  void receive_pkt(Packet* pkt);

private:
  FIFO<Flit*>** m_inj_que;
  FIFO<Flit*>** m_eje_rob;
  Coord m_coord;
  Flit** m_link_flits;

  std::set<const Packet*> m_inflight_pkts;
};

#endif /* __NODE_H__ */
