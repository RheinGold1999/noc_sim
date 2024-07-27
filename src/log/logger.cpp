// #include <filesystem>

#include "logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "config/global_config.h"

namespace fs = std::filesystem;

Logger::Logger() :
  m_model(nullptr),
  m_spdlogger(nullptr)
{}

Logger::Logger(const ModelBase* model) :
  m_model(model),
  m_spdlogger(nullptr)
{}

Logger::Logger(const ModelBase* model, spd_logger_t spdlogger) :
  m_model(model),
  m_spdlogger(spdlogger)
{}

Logger::Logger(const ModelBase* model, spd_logger_t spdlogger, const fs::path& path) :
  m_model(model),
  m_spdlogger(spdlogger),
  m_path(path)
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

std::list<Logger*> Logger::s_logger_list;

const Logger*
Logger::create_logger(const ModelBase* model)
{
  fs::path dir_path = fs::current_path().parent_path().parent_path().append("log");
  // std::cout << "dir_path: " << dir_path << std::endl;
  if (!fs::exists(dir_path)) {
    fs::create_directories(dir_path);
  }
  fs::path log_path = dir_path.append(model->base_name() + ".log");
  // std::cout << "log_path: " << log_path << std::endl;
  auto spd_logger = spdlog::basic_logger_mt(model->base_name(), log_path.string(), true);
  spd_logger->set_pattern("[%L] %v");
  spd_logger->set_level(spdlog::level::trace);
  spd_logger->set_level(static_cast<spdlog::level::level_enum>((int)GlobalConfig::top_dbg_lvl));
  spd_logger->flush_on(spdlog::level::err);
  Logger* logger =  new Logger(model, spd_logger, log_path);
  std::cout << "create log file: " << log_path << std::endl;
  s_logger_list.emplace_back(logger);
  return logger;
}

const Logger*
Logger::inherit_from(const ModelBase* parent, const ModelBase* child)
{
  return new Logger(
    child,
    parent->m_logger->m_spdlogger,
    parent->m_logger->m_path
  );
}


