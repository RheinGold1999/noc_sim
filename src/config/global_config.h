#ifndef __GLOBAL_CONFIG_H__
#define __GLOBAL_CONFIG_H__

#include <regex>
#include <cassert>

#include "config/config_base.h"

#define DEMO(args...) std::cout << #args << " = " << args << std::endl
#define COUT(args...) std::cout << args << std::endl

struct GlobalConfig
  : public ConfigBase
{
  static param_map_type param_map;

  // ---------------------------------------------------------------------------
  // Log
  // ---------------------------------------------------------------------------
  DECL_PARAM(std::string, log_file)
  DECL_PARAM(int, top_dbg_lvl)

  // ---------------------------------------------------------------------------
  // Simulation
  // ---------------------------------------------------------------------------
  DECL_PARAM(int, max_tick)

};


#endif  /* __GLOBAL_CONFIG_H__ */
