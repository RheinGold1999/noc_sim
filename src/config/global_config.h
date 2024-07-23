#ifndef __GLOBAL_CONFIG_H__
#define __GLOBAL_CONFIG_H__

#include "config/config_base.h"

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
