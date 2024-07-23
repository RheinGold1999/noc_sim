#include "noc/double_ring.h"
#include "noc/node_router.h"
#include "noc/bridge_router.h"
#include "noc/link.h"
#include "noc/data_type.h"
#include "config/noc_config.h"
#include "log/logger.h"

DoubleRing::DoubleRing(const ModelBase* parent, const std::string& name)
  : ModuleBase(parent, name)
{
  m_node_router_num = NocConfig::network_lvl_0_num * 2;
  m_bridge_router_num = 2;
  m_link_num = m_node_router_num + m_bridge_router_num;

  m_node_routers = new NodeRouter* [m_node_router_num];
  m_bridge_routers = new BridgeRouter* [m_bridge_router_num];
  m_links = new Link* [m_link_num];

  std::stringstream os;
  for (int i = 0; i < m_node_router_num; ++i) {
    Coord coord(i);
    os.clear();
    os << "node_router_" << i;
    m_node_routers[i] = new NodeRouter(this, os.str(), coord);
  }

  for (int i = 0; i < m_bridge_router_num; ++i) {
    NodeAddr addr(0, 0, i, NodeAddr::MASKED);
    os.clear();
    os << "bridge_router_" << i;
    m_bridge_routers[i] = new BridgeRouter(this, os.str(), addr);
  }

  for (int i = 0; i < m_link_num; ++i) {

  }
}

DoubleRing::~DoubleRing()
{

}

void
DoubleRing::transfer()
{

}

void
DoubleRing::process()
{

}

void
DoubleRing::update()
{

}
