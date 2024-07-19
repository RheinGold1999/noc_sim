#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <sstream>

#include "spdlog/spdlog.h"

#include "model_utils/model_base.h"
#include "model_utils/top.h"
#include "model_utils/simulator.h"
#include "log/src_loc.h"

/**
 * @brief `Logger` serves as a wrapper of `spdlog::logger` to provide customized
 *        format for ESL simulation. Usually the parent passes its `spdlog::logger` 
 *        to its child, so not all the Models have their own `spdlog::logger`, 
 *        but they both have the `Logger` object.
 */
class Logger
{
  friend class ModelBase;
  friend class Simulator;

  typedef std::shared_ptr<spdlog::logger> logger_t;

public:
  Logger();
  Logger(const ModelBase* model);
  Logger(const ModelBase* model, logger_t logger);

  void set_level(int lvl) const;

private:
  template<typename... Args>
  inline void _log(
    SrcLoc src_loc,
    spdlog::level::level_enum lvl,
    const std::string& fmt,
    Args&& ...args
  ) const {
    std::ostringstream os;
    if (m_model) {
      os << "[" << Simulator::curr_tick() << ":" << m_model->get_stage() << "] "
         << "[" << src_loc.file << ":" << src_loc.line << "] "
         << "[" << src_loc.func << "] [" << m_model->base_name() << "] ";
    } else {
      os << "[" << Simulator::curr_tick() << ":-] "
         << "[" << src_loc.file << ":" << src_loc.line << "] "
         << "[" << src_loc.func << "] ";
    }
    os << fmt;
    m_spdlogger->log(lvl, os.str(), args...);
  }

public:
  template<typename... Args>
  inline void trace(
    SrcLoc src_loc,
    const std::string& fmt,
    Args&& ...args
  ) const {
    _log(src_loc, spdlog::level::trace, fmt, args...);
  }

  template<typename... Args>
  inline void debug(
    SrcLoc src_loc,
    const std::string& fmt,
    Args&& ...args
  ) const {
    _log(src_loc, spdlog::level::debug, fmt, args...);
  }

  template<typename... Args>
  inline void info(
    SrcLoc src_loc,
    const std::string& fmt,
    Args&& ...args
  ) const {
    _log(src_loc, spdlog::level::info, fmt, args...);
  }

  template<typename... Args>
  inline void warn(
    SrcLoc src_loc,
    const std::string& fmt,
    Args&& ...args
  ) const {
    _log(src_loc, spdlog::level::warn, fmt, args...);
  }

  template<typename... Args>
  inline void error(
    SrcLoc src_loc,
    const std::string& fmt,
    Args&& ...args
  ) const {
    _log(src_loc, spdlog::level::err, fmt, args...);
  }

  template<typename... Args>
  inline void critical(
    SrcLoc src_loc,
    const std::string& fmt,
    Args&& ...args
  ) const {
    _log(src_loc, spdlog::level::critical, fmt, args...);
  }

  bool has_spdlogger() const;

private:
  const ModelBase* m_model;
  mutable logger_t m_spdlogger;
  
private:
  static const Logger* create_logger(const ModelBase* model);
  static const Logger* inherit_from(const ModelBase* parent, const ModelBase* child);
};

// -----------------------------------------------------------------------------
// Convenient Logger Macros for Global Use
// -----------------------------------------------------------------------------

#define _TRACE(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  Top::logger->trace(src_loc, fmt, ##args); \
} while (0);

#define _DEBUG(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  Top::logger->debug(src_loc, fmt, ##args); \
} while (0);

#define _INFO(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  Top::logger->info(src_loc, fmt, ##args); \
} while (0);

#define _WARN(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  Top::logger->warn(src_loc, fmt, ##args); \
} while (0);

#define _ERROR(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  Top::logger->error(src_loc, fmt, ##args); \
} while (0);

#define _CRITICAL(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  Top::logger->critical(src_loc, fmt, ##args); \
} while (0);

// -----------------------------------------------------------------------------
// Convenient Logger Macros for ModelBase
// -----------------------------------------------------------------------------

#define TRACE(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  m_logger->trace(src_loc, fmt, ##args); \
} while (0);

#define DEBUG(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  m_logger->debug(src_loc, fmt, ##args); \
} while (0);

#define INFO(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  m_logger->info(src_loc, fmt, ##args); \
} while (0);

#define WARN(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  m_logger->warn(src_loc, fmt, ##args); \
} while (0);

#define ERROR(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  m_logger->error(src_loc, fmt, ##args); \
} while (0);

#define CRITICAL(fmt, args...) \
do { \
  SrcLoc src_loc{__FILE__, __LINE__, __FUNCTION__}; \
  m_logger->critical(src_loc, fmt, ##args); \
} while (0);


#endif /* __LOGGER_H__ */
