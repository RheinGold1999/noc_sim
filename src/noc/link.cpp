#include "noc/link.h"
#include "noc/node_router.h"
#include "noc/bridge_router.h"
#include "model_utils/port.h"
#include "config/noc_config.h"
#include "log/logger.h"

Link::Link(const ModelBase* parent, const std::string& name)
  : ModuleBase(parent, name)
{
  ASSERT(NocConfig::ring_width % 2 == 0);

  link_o = new StreamPortOut<Flit*>* [NocConfig::ring_width];
  link_i = new StreamPortIn<Flit*>* [NocConfig::ring_width];

  m_pipeline_regs = new Flit* [NocConfig::ring_width];

  std::ostringstream os;
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    os.str("");
    os << "link_o_" << i;
    link_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.str("");
    os << "link_i_" << i;
    link_i[i] = new StreamPortIn<Flit*>(this, os.str());

    m_pipeline_regs[i] = nullptr;
  }

  INFO("created");
}

Link::~Link()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    delete link_o[i];
    delete link_i[i];
  }

  delete [] link_o;
  delete [] link_i;

  delete [] m_pipeline_regs;
}

void
Link::transfer()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (link_i[i]->can_read()) {
      m_pipeline_regs[i] = link_i[i]->read();
    }
  }
}

void
Link::process()
{

}

void
Link::update()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (m_pipeline_regs[i]) {
      ASSERT(link_o[i]->can_write());
      link_o[i]->write(m_pipeline_regs[i]);
      m_pipeline_regs[i] = nullptr;
    }
  }
}

void
Link::connect(NodeRouter* up, NodeRouter* dn)
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (i % 2 == 0) {
      // clockwise bind
      up->inj_o[i]->bind(link_i[i]);
      link_o[i]->bind(dn->eje_i[i]);
    } else {
      // anti-clockwise bind
      dn->inj_o[i]->bind(link_i[i]);
      link_o[i]->bind(up->eje_i[i]);
    }
  }
}

void
Link::connect(BridgeRouter* up, BridgeRouter* dn)
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (i % 2 == 0) {
      // clockwise bind
      up->glb_inj_o[i]->bind(link_i[i]);
      link_o[i]->bind(dn->glb_eje_i[i]);
    } else {
      // anti-clockwise bind
      dn->glb_inj_o[i]->bind(link_i[i]);
      link_o[i]->bind(up->glb_eje_i[i]);
    }
  }
}

void
Link::connect(NodeRouter* up, BridgeRouter* dn)
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (i % 2 == 0) {
      // clockwise bind
      up->inj_o[i]->bind(link_i[i]);
      link_o[i]->bind(dn->loc_eje_i[i]);
    } else {
      // anti-clockwise bind
      dn->loc_inj_o[i]->bind(link_i[i]);
      link_o[i]->bind(up->eje_i[i]);
    }
  }
}

void
Link::connect(BridgeRouter* up, NodeRouter* dn)
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (i % 2 == 0) {
      // clockwise bind
      up->loc_inj_o[i]->bind(link_i[i]);
      link_o[i]->bind(dn->eje_i[i]);
    } else {
      // anti-clockwise bind
      dn->inj_o[i]->bind(link_i[i]);
      link_o[i]->bind(up->loc_eje_i[i]);
    }
  }
}


void
Link::connect(Router* up, Router* dn)
{
  NodeRouter* node_up = dynamic_cast<NodeRouter*>(up);
  NodeRouter* node_dn = dynamic_cast<NodeRouter*>(dn);
  BridgeRouter* bridge_up = dynamic_cast<BridgeRouter*>(up);
  BridgeRouter* bridge_dn = dynamic_cast<BridgeRouter*>(dn);

  m_addr_up = up->get_addr();
  m_addr_dn = dn->get_addr();
  INFO("connect {} <-> {}", m_addr_up.to_str(), m_addr_dn.to_str());

  if (node_up && node_dn) {
    connect(node_up, node_dn);
  } else if (bridge_up && bridge_dn) {
    connect(bridge_up, bridge_dn);
  } else if (bridge_up && node_dn) {
    connect(bridge_up, node_dn);
  } else if (node_up && bridge_dn) {
    connect(node_up, bridge_dn);
  } else {
    ERROR("wrong router type!!!");
    abort();
  }
}

