#include "model_utils/top.h"
#include "log/logger.h"

Top::Top(): ModelBase(nullptr, "top")
{
  m_type = ModelType::TOP;
}

void
Top::transfer() {}

void
Top::process() {}

void
Top::update() {}

Top*
Top::instance()
{
  static Top top;
  return &top;
}

const Logger* Top::logger = Top::instance()->m_logger;
