#include "config/global_config.h"
#include "log/logger.h"
#include <cassert>

// -----------------------------------------------------------------------------
// Add parameters in this area
// -----------------------------------------------------------------------------

INIT_PARAM(GlobalConfig, int, max_tick, 100, "maximum of ticks in simulation")
INIT_PARAM(GlobalConfig, int, top_dbg_lvl, 0, "top debug level")


GlobalConfig::param_map_type GlobalConfig::param_map = {
  MAP_PARAM(GlobalConfig, max_tick)
  MAP_PARAM(GlobalConfig, top_dbg_lvl)
};

