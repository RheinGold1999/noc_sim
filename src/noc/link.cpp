#include "noc/link.h"
#include "model_utils/port.h"
#include "config/noc_config.h"
#include "log/logger.h"

Link::Link(const ModelBase* parent, const std::string& name)
  : ModuleBase(parent, name)
{
  link_o = new StreamPortOut<Flit*>* [NocConfig::ring_width];
  link_i = new StreamPortIn<Flit*>* [NocConfig::ring_width];

  m_pipeline_regs = new Flit* [NocConfig::ring_width];

  std::ostringstream os;
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    os.clear();
    os << "link_o_" << i;
    link_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.clear();
    os << "link_i_" << i;
    link_i[i] = new StreamPortIn<Flit*>(this, os.str());

    m_pipeline_regs[i] = nullptr;
  }
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
