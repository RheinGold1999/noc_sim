#ifndef __LINK_H__
#define __LINK_H__

#include "model_utils/module_base.h"
#include "noc/data_type.h"

template <typename T>
class StreamPortOut;

template <typename T>
class StreamPortIn;

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

private:
  Flit** m_pipeline_regs;
};

#endif /* __LINK_H__ */
