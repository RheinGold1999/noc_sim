#ifndef __DOUBLE_RING_H__
#define __DOUBLE_RING_H__

#include "model_utils/module_base.h"

class NodeRouter;
class BridgeRouter;
class Link;

class DoubleRing
  : public ModuleBase
{
public:
  DoubleRing(const ModelBase* parent, const std::string& name);
  ~DoubleRing();

  void transfer() override;
  void process() override;
  void update() override;

public:
  NodeRouter** m_node_routers;
  BridgeRouter** m_bridge_routers;
  Link** m_links;

  int m_node_router_num;
  int m_bridge_router_num;
  int m_link_num;
};

#endif /* __DOUBLE_RING_H__ */
