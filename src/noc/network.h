#ifndef __NETWORK_H__
#define __NETWORK_H__

#include "model_utils/module_base.h"

class Network
  : public ModuleBase
{
public:
  Network(const ModelBase* parent, const std::string& name);

  void finalize() override;
};

#endif  /* __NETWORK_H__ */
