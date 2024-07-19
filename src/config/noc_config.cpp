#include "config/noc_config.h"

INIT_PARAM(NocConfig, int, hellow, 0, "are")
INIT_PARAM(NocConfig, int, world, 1, "are")
INIT_PARAM(NocConfig, int, are, 0xf, "are")
INIT_PARAM(NocConfig, double, you, 3.14, "you")
INIT_PARAM(NocConfig, double, ok, 2.17, "ok")

std::map<std::string, ParameterBase*> NocConfig::param_map = {
  MAP_PARAM(NocConfig, hellow)
  MAP_PARAM(NocConfig, world)
  MAP_PARAM(NocConfig, are)
  MAP_PARAM(NocConfig, you)
  MAP_PARAM(NocConfig, ok)
};
