#include "noc/node_router.h"
#include "noc/node.h"
#include "model_utils/port.h"
#include "model_utils/fifo.h"
#include "config/noc_config.h"

NodeRouter::NodeRouter(
  const ModelBase* parent, 
  const std::string& name,
  const Coord& coord
)
  : Router(parent, name),
    m_coord(coord)
{
  ASSERT(NocConfig::ring_width % 2 == 0);

  inj_o = new StreamPortOut<Flit*>* [NocConfig::ring_width];
  eje_i = new StreamPortIn<Flit*>* [NocConfig::ring_width];

  node_o = new StreamPortOut<Flit*>* [NocConfig::ring_width];
  node_i = new StreamPortIn<Flit*>* [NocConfig::ring_width];

  m_inj_que = new FIFO<Flit*>* [NocConfig::ring_width];
  m_eje_que = new FIFO<Flit*>* [NocConfig::ring_width];

  m_arb_flits = new Flit* [NocConfig::ring_width];

  std::ostringstream os;
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    os.str("");
    os << "inj_o_" << i;
    inj_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.str("");
    os << "eje_i_" << i;
    eje_i[i] = new StreamPortIn<Flit*>(this, os.str());

    os.str("");
    os << "node_o_" << i;
    node_o[i] = new StreamPortOut<Flit*>(this, os.str());

    os.str("");
    os << "node_i_" << i;
    node_i[i] = new StreamPortIn<Flit*>(this, os.str());

    os.str("");
    os << "inj_que_" << i;
    m_inj_que[i] = new FIFO<Flit*>(this, os.str(), NocConfig::node_inj_que_depth);

    os.str("");
    os << "eje_que_" << i;
    m_eje_que[i] = new FIFO<Flit*>(this, os.str(), NocConfig::node_eje_que_depth);

    m_arb_flits[i] = nullptr;
  }

  INFO("created: {}", m_coord.to_str());
}

NodeRouter::~NodeRouter()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    delete inj_o[i];
    delete eje_i[i];
    delete m_inj_que[i];
    delete m_eje_que[i];
  }

  delete [] inj_o;
  delete [] eje_i;
  delete [] m_inj_que;
  delete [] m_eje_que;

  delete [] m_arb_flits;
}

bool
NodeRouter::is_this_dst(const Flit* flit) const
{
  Coord dst = flit->get_dst();
  if (dst.is_equal(this->m_coord)) {
    return true;  
  }
  return false;
}

void
NodeRouter::connect_node(Node* node)
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    node_o[i]->bind(node->flit_i[i]);
    node_i[i]->bind(node->flit_o[i]);
  }
}

void
NodeRouter::transfer()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (eje_i[i]->can_read()) {
      Flit* flit = eje_i[i]->read();
      if (is_this_dst(flit)) {
        DEBUG("rcv flit: {}", flit->to_str());
        if (m_eje_que[i]->can_write()) {
          m_eje_que[i]->write(flit);
          flit->get_pkt()->m_arrived_flits_num++;
          DEBUG("push to eje_que: {}", flit->to_str());
        } else {
          // TODO: write to retransmit buffer
        }
      } else {
        m_arb_flits[i] = flit;
        DEBUG("forward flit: {}", flit->to_str());
      }
    }
  }

  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (node_i[i]->can_read() && m_inj_que[i]->can_write()) {
      Flit* flit = node_i[i]->read();
      DEBUG("get flit from node, {}", flit->to_str());
      m_inj_que[i]->write(flit);
    }
  }
}

void
NodeRouter::process()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (!m_arb_flits[i] && m_inj_que[i]->can_read()) {
      Flit* flit = m_inj_que[i]->read();
      DEBUG("arb flit to ring: {}", flit->to_str());
      m_arb_flits[i] = flit;
    }
  }
}

void
NodeRouter::update()
{
  for (int i = 0; i < NocConfig::ring_width; ++i) {
    ASSERT(inj_o[i]->can_write());
    if (m_arb_flits[i]) {
      inj_o[i]->write(m_arb_flits[i]);
      DEBUG("inj flit to ring, {}", m_arb_flits[i]->to_str());
      m_arb_flits[i] = nullptr;
    }
  }

  for (int i = 0; i < NocConfig::ring_width; ++i) {
    if (node_o[i]->can_write() && m_eje_que[i]->can_read()) {
      node_o[i]->write(m_eje_que[i]->read());
    }
  }
}

NodeAddr
NodeRouter::get_addr() const
{
  return m_coord.get_addr();
}

Coord
NodeRouter::get_coord() const
{
  return m_coord;
}

