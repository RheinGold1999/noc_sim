#include "noc/double_ring.h"
#include "noc/node_router.h"
#include "noc/bridge_router.h"
#include "noc/node.h"
#include "noc/link.h"
#include "noc/data_type.h"
#include "config/noc_config.h"
#include "log/logger.h"

DoubleRing::DoubleRing(const ModelBase* parent, const std::string& name) :
  ModuleBase(parent, name)
{
  // ASSERT(
  //   NocConfig::network_lvl_0_num == (8 + 1) ||
  //   NocConfig::network_lvl_0_num == (12 + 1) ||
  //   NocConfig::network_lvl_0_num == (16 + 1)
  // );
  ASSERT(NocConfig::network_lvl_1_num == 2);

  m_node_num = get_node_num();
  m_bridge_num = get_bridge_num();
  m_link_num = get_link_num();

  m_routers = new Router* [m_node_num] {nullptr};
  m_node_links = new Link* [m_node_num] {nullptr};
  m_bridge_links = new Link* [m_bridge_num] {nullptr};
  m_nodes = new Node* [m_node_num] {nullptr};

  // ---------------------------------------------------------------------------
  // Define the bridge index
  // ---------------------------------------------------------------------------
  m_bridge_idx_vec = DoubleRing::get_bridge_idx_vec();

  // ---------------------------------------------------------------------------
  // Instantiate all the NodeRouters, the BridgeRouters and the Links
  // ---------------------------------------------------------------------------
  std::stringstream os;
  for (int i = 0; i < m_node_num; ++i) {
    Coord coord(i);
    bool is_bridge = 
      std::find(m_bridge_idx_vec.begin(), m_bridge_idx_vec.end(), i) != m_bridge_idx_vec.end();
    if (is_bridge) {
      os.str("");
      os << "bridge_router_" << i;
      m_routers[i] = new BridgeRouter(this, os.str(), coord);
      dynamic_cast<BridgeRouter*>(m_routers[i])->set_addr(0, NodeAddr::MASKED);
      m_nodes[i] = nullptr;
    } else {
      os.str("");
      os << "node_router_" << i;
      m_routers[i] = new NodeRouter(this, os.str(), coord);
      os.str("");
      os << "node_" << i;
      m_nodes[i] = new Node(this, os.str(), coord);
      dynamic_cast<NodeRouter*>(m_routers[i])->connect_node(m_nodes[i]);
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
    delete m_nodes[i];
  }
  delete [] m_routers;
  delete [] m_nodes;

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

int
DoubleRing::get_node_num()
{
  static int node_num = NocConfig::network_lvl_0_num * NocConfig::network_lvl_1_num;
  return node_num;
}

int
DoubleRing::get_bridge_num()
{
  static int bridge_num = NocConfig::network_lvl_1_num * NocConfig::network_lvl_0_bridge_num;
  return bridge_num;
}

int
DoubleRing::get_link_num()
{
  static int link_num = get_node_num() + get_bridge_num();
  return link_num;
}

const std::vector<int>&
DoubleRing::get_bridge_idx_vec()
{
  const size_t bridge_num = NocConfig::network_lvl_1_num * NocConfig::network_lvl_0_bridge_num;
  std::vector<int> bridge_idx_vec;
  for (int i = 1; i <= bridge_num; ++i) {
    bridge_idx_vec.emplace_back(
      i * (NocConfig::network_lvl_0_num / NocConfig::network_lvl_0_bridge_num) - 1
    );
  }
  static const std::vector<int> static_const_bridge_idx_vec(bridge_idx_vec);
  return static_const_bridge_idx_vec;
}

Coord
DoubleRing::gen_random_dst(Coord src)
{
  int dst_id = 0;
  do {
    dst_id = rand() % (get_node_num());
  } while (
    dst_id == src.get_id() || 
    std::find(get_bridge_idx_vec().begin(), get_bridge_idx_vec().end(), dst_id) != get_bridge_idx_vec().end()
  );

  return Coord(dst_id);
}
