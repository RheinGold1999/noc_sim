#include "model_utils/top.h"
#include "log/logger.h"
#include "config/global_config.h"

const Logger* Top::logger = Top::instance()->m_logger;

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

void
Top::elaborate()
{
  m_logger->set_level((int)GlobalConfig::top_dbg_lvl);
  int lvl = (int) m_logger->m_spdlogger->level();
  CRITICAL("Top logger level is set to: {}", Logger::lvl_str[lvl]);
}

