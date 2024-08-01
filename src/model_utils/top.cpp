#include "model_utils/top.h"
#include "model_utils/port.h"
#include "log/logger.h"
#include "config/global_config.h"

const Logger* Top::logger = Top::instance()->m_logger;
bool all_bound = true;

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
  // update logger level
  m_logger->set_level((int)GlobalConfig::top_dbg_lvl);
  int lvl = (int) m_logger->m_spdlogger->level();
  CRITICAL("Top logger level is set to: {}", Logger::lvl_str[lvl]);

  // check if all ports are bound
  if (!m_unbound_port_list.empty()) {
    for (auto p : m_unbound_port_list) {
      CRITICAL("unbound port: {}", p->full_name());
    }
    abort();
  }
}

void
Top::register_unbound_port(const PortBase* port)
{
  m_unbound_port_list.emplace_back(port);
}

