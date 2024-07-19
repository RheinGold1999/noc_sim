// #include <filesystem>

#include "logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "config/global_config.h"

Logger::Logger()
  : m_model(nullptr),
    m_spdlogger(nullptr)
{}

Logger::Logger(const ModelBase* model)
  : m_model(model),
    m_spdlogger(nullptr)
{}

Logger::Logger(const ModelBase* model, logger_t spdlogger)
  : m_model(model),
    m_spdlogger(spdlogger)
{}

bool
Logger::has_spdlogger() const
{
  return m_spdlogger != nullptr;
}

void
Logger::set_level(int lvl) const
{
  assert(0 <= lvl && lvl <= 6);
  if (m_spdlogger != nullptr) {
    m_spdlogger->set_level(static_cast<spdlog::level::level_enum>(lvl));
  }
}

const Logger*
Logger::create_logger(const ModelBase* model)
{
  std::string log_name = std::string("../../log/") + model->base_name() + ".log";
  auto logger = spdlog::basic_logger_mt(model->base_name(), log_name, true);
  logger->set_pattern("[%L] %v");
  logger->set_level(spdlog::level::trace);
  logger->set_level(static_cast<spdlog::level::level_enum>((int)GlobalConfig::top_dbg_lvl));
  logger->flush_on(spdlog::level::err);
  return new Logger(model, logger);
}

const Logger*
Logger::inherit_from(const ModelBase* parent, const ModelBase* child)
{
  const Logger* logger = new Logger(child);
  logger->m_spdlogger = parent->m_logger->m_spdlogger;
  return logger;
}


