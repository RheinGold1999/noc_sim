#ifndef __LINK_H__
#define __LINK_H__

#include "model_utils/module_base.h"
#include "noc/data_type.h"

template <typename T>
class StreamPortOut;

template <typename T>
class StreamPortIn;

class NodeRouter;

class BridgeRouter;

class Router;

class Link
  : public ModuleBase
{
public:
  StreamPortOut<Flit*>** link_o;
  StreamPortIn<Flit*>** link_i;

public:
  Link(const ModelBase* parent, const std::string& name);
  ~Link();

  void transfer() override;
  void process() override;
  void update() override;

  void connect(NodeRouter* up, NodeRouter* dn);
  void connect(BridgeRouter* up, BridgeRouter* dn);
  void connect(NodeRouter* up, BridgeRouter* dn);
  void connect(BridgeRouter* up, NodeRouter* dn);
  void connect(Router* up, Router* dn);

private:
  Flit** m_pipeline_regs;
  Coord m_coord_up;  // in clockwise direction
  Coord m_coord_dn;  // in clockwise direction
};

#endif /* __LINK_H__ */
