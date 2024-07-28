#include "noc/node.h"
#include "model_utils/port.h"
#include "config/noc_config.h"

Node::Node(const ModelBase* parent, const std::string& name) :
  ModuleBase(parent, name)
{
  m_issue_que.clear();
  m_inflight_map.clear();

  flit_o = new StreamPortOut<Flit*>* [NocConfig::ring_width];
  flit_i = new StreamPortIn<Flit*>* [NocConfig::ring_width];

  std::ostringstream os;
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    os.str("");
    os << "flit_o_" << i;
    flit_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.str("");
    os << "flit_i_" << i;
    flit_i[i] = new StreamPortIn<Flit*>(this, os.str());
  }

  INFO("created");
}

Node::~Node()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    delete flit_o[i];
    delete flit_i[i];
  }
  delete [] flit_o;
  delete [] flit_i;
}

void
Node::transfer()
{

}

void
Node::process()
{

}

void
Node::update()
{

}

void
Node::gen_req_pkt()
{

}

void
Node::gen_rsp_pkt()
{
  
}
