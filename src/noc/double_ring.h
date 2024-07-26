#ifndef __DOUBLE_RING_H__
#define __DOUBLE_RING_H__

#include <vector>

#include "model_utils/module_base.h"

class Router;
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
  Router** m_routers;
  Link** m_node_links;
  Link** m_bridge_links;

  int m_node_num;
  int m_bridge_num;
  int m_link_num;
  std::vector<int> m_bridge_idx_vec;
};

#endif /* __DOUBLE_RING_H__ */
