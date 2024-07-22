#ifndef __BRIDGE_ROUTER_H__
#define __BRIDGE_ROUTER_H__

#include "model_utils/module_base.h"
#include "noc/data_type.h"

template <typename T>
class StreamPortOut;

template <typename T>
class StreamPortIn;

template <typename T>
class FIFO;

class BridgeRouter : public ModuleBase
{
public:
  StreamPortOut<Flit*>** loc_inj_o;
  StreamPortIn<Flit*>** loc_eje_i;

  StreamPortOut<Flit*>** glb_inj_o;
  StreamPortIn<Flit*>** glb_eje_i;

public:
  BridgeRouter(
    const ModelBase* parent,
    const std::string& name,
    const NodeAddr& addr
  );
  ~BridgeRouter();

  void transfer() override;
  void process() override;
  void update() override;

  bool is_matched(const Flit* flit);

private:
  FIFO<Flit*>** m_glb_inj_que;
  FIFO<Flit*>** m_glb_eje_que;
  FIFO<Flit*>** m_loc_inj_que;
  FIFO<Flit*>** m_loc_eje_que;
  NodeAddr m_addr;

};

#endif /* __BRIDGE_ROUTER_H__ */