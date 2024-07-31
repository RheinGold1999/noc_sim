#ifndef __NODE_ROUTER_H__
#define __NODE_ROUTER_H__

#include <set>

#include "noc/router.h"
#include "noc/data_type.h"

template <typename T>
class StreamPortOut;

template <typename T>
class StreamPortIn;

template <typename T>
class FIFO;

class Node;

class NodeRouter
  : public Router
{
public:
  StreamPortOut<Flit*>** inj_o;
  StreamPortIn<Flit*>**  eje_i;

  StreamPortOut<Flit*>** node_o;
  StreamPortIn<Flit*>** node_i;

public:
  NodeRouter(
    const ModelBase* parent, 
    const std::string& name,
    const Coord& coord
  );
  ~NodeRouter();

  void transfer() override;
  void process() override;
  void update() override;

  bool is_this_dst(const Flit* flit) const;
  void connect_node(Node* node);

  NodeAddr get_addr() const override;
  Coord get_coord() const override;

private:
  FIFO<Flit*>** m_inj_que;
  FIFO<Flit*>** m_eje_que;
  Coord m_coord;
  Flit** m_arb_flits;

};

#endif /* __NODE_H__ */
