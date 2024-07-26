#include "noc/double_ring.h"
#include "noc/node_router.h"
#include "noc/bridge_router.h"
#include "noc/link.h"
#include "noc/data_type.h"
#include "config/noc_config.h"
#include "log/logger.h"

DoubleRing::DoubleRing(const ModelBase* parent, const std::string& name) :
  ModuleBase(parent, name)
{
  ASSERT(
    NocConfig::network_lvl_0_num == (8 + 1) ||
    NocConfig::network_lvl_0_num == (12 + 1) ||
    NocConfig::network_lvl_0_num == (16 + 1)
  );
  ASSERT(NocConfig::network_lvl_1_num == 2);

  m_node_num = NocConfig::network_lvl_0_num * NocConfig::network_lvl_1_num;
  m_bridge_num = NocConfig::network_lvl_1_num;
  m_link_num = m_node_num + m_bridge_num;

  m_routers = new Router* [m_node_num] {nullptr};
  m_node_links = new Link* [m_node_num] {nullptr};
  m_bridge_links = new Link* [m_bridge_num] {nullptr};

  // ---------------------------------------------------------------------------
  // Define the bridge index
  // ---------------------------------------------------------------------------
  m_bridge_idx_vec.clear();
  for (int i = 0; i < m_bridge_num; ++i) {
    m_bridge_idx_vec.emplace_back(
      i * NocConfig::network_lvl_0_num + NocConfig::network_lvl_0_num / 2
    );
  }

  // ---------------------------------------------------------------------------
  // Instantiate all the NodeRouters, the BridgeRouters and the Links
  // ---------------------------------------------------------------------------
  std::stringstream os;
  for (int i = 0; i < m_node_num; ++i) {
    Coord coord(i);
    os.str("");
    bool is_bridge = 
      std::find(m_bridge_idx_vec.begin(), m_bridge_idx_vec.end(), i) != m_bridge_idx_vec.end();
    if (is_bridge) {
      os << "bridge_router_" << i;
      m_routers[i] = new BridgeRouter(this, os.str(), coord);
      NodeAddr bridge_addr(0, 0, (i / NocConfig::network_lvl_0_num), NodeAddr::MASKED);
      dynamic_cast<BridgeRouter*>(m_routers[i])->set_addr(bridge_addr);
    } else {
      os << "node_router_" << i;
      m_routers[i] = new NodeRouter(this, os.str(), coord);
    }
  }

  for (int i = 0; i < m_node_num; ++i) {
    os.str("");
    os << "node_link_" << i;
    m_node_links[i] = new Link(this, os.str());
  }

  for (int i = 0; i < m_bridge_num; ++i) {
    os.str("");
    os << "bridge_link_" << i;
    m_bridge_links[i] = new Link(this, os.str());
  }

  // ---------------------------------------------------------------------------
  // Connect all the nodes by the m_node_links
  // ---------------------------------------------------------------------------
  for (int i = 0; i < m_node_num; ++i) {
    if ((i + 1) % NocConfig::network_lvl_0_num != 0) {
      // current node IS NOT the last one in its local ring
      m_node_links[i]->connect(m_routers[i], m_routers[i + 1]);
    } else {
      // current node IS the last one in its local ring
      m_node_links[i]->connect(m_routers[i], m_routers[i - NocConfig::network_lvl_0_num + 1]);
    }
  }
  
  // ---------------------------------------------------------------------------
  // Connect all the bridges by the m_bridge_links
  // ---------------------------------------------------------------------------
  for (int i = 0; i < m_bridge_num; ++i) {
    if ((i + 1) % m_bridge_num != 0) {
      // current bridge IS NOT the last one in its global ring
      m_bridge_links[i]->connect(
        m_routers[m_bridge_idx_vec[i]],
        m_routers[m_bridge_idx_vec[i + 1]]
      );
    } else {
      // current bridge IS the last one in its global ring
      m_bridge_links[i]->connect(
        m_routers[m_bridge_idx_vec[i]],
        m_routers[m_bridge_idx_vec[i - m_bridge_num + 1]]
      );
    }
  }
  
}

DoubleRing::~DoubleRing()
{
  for (int i = 0; i < m_node_num; ++i) {
    delete m_routers[i];
  }
  delete [] m_routers;

  for (int i = 0; i < m_node_num; ++i) {
    delete m_node_links[i];
  }
  delete [] m_node_links;

  for (int i = 0; i < m_bridge_num; ++i) {
    delete m_bridge_links[i];
  }
  delete [] m_bridge_links;
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
