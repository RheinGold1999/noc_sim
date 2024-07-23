#include "noc/double_ring.h"
#include "noc/node_router.h"
#include "noc/bridge_router.h"
#include "noc/link.h"
#include "noc/data_type.h"
#include "config/noc_config.h"

DoubleRing::DoubleRing(const ModelBase* parent, const std::string& name)
  : ModuleBase(parent, name)
{
  m_node_router_num = NocConfig::network_lvl_0_num * 2;
  m_bridge_router_num = 2;
  m_link_num = NocConfig::network_lvl_0_num * 2 + 2;

  m_node_routers = new NodeRouter* [m_node_router_num];
  m_bridge_routers = new BridgeRouter* [m_bridge_router_num];
  m_links = new Link* [m_link_num];

  for (int i = 0; i < m_node_router_num; ++i) {
    
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
