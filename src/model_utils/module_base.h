#ifndef __MODULE_BASE_H__
#define __MODULE_BASE_H__

#include "model_utils/model_base.h"

class ModuleBase
  : public ModelBase
{
public:
  ModuleBase(const ModelBase* parent, const std::string& name)
    : ModelBase(parent, name)
  {
    m_type = ModelType::MODULE;
  }
};

#endif  /* __MODULE_BASE_H__ */
