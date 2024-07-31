#ifndef __DOUBLE_RING_H__
#define __DOUBLE_RING_H__

#include <vector>

#include "noc/network.h"
#include "noc/data_type.h"

class Router;
class Link;
class Node;

class DoubleRing
  : public Network
{
public:
  DoubleRing(const ModelBase* parent, const std::string& name);
  ~DoubleRing();

  void transfer() override;
  void process() override;
  void update() override;

  static const std::vector<int>& get_bridge_idx_vec();
  static int get_node_num();
  static int get_bridge_num();
  static int get_link_num();
  static Coord gen_random_dst(Coord src);

public:
  Router** m_routers;
  Link** m_node_links;
  Link** m_bridge_links;
  Node** m_nodes;

  int m_node_num;
  int m_bridge_num;
  int m_link_num;
  std::vector<int> m_bridge_idx_vec;
};

#endif /* __DOUBLE_RING_H__ */
