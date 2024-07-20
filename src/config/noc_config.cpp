#include "config/noc_config.h"

INIT_PARAM(NocConfig, int, network_lvl_0_num, 0, "network level 0 number")
INIT_PARAM(NocConfig, int, network_lvl_1_num, 0, "network level 1 number")
INIT_PARAM(NocConfig, int, network_lvl_2_num, 0, "network level 2 number")
INIT_PARAM(NocConfig, int, network_lvl_3_num, 0, "network level 3 number")

std::map<std::string, ParameterBase*> NocConfig::param_map = {
  MAP_PARAM(NocConfig, network_lvl_0_num)
  MAP_PARAM(NocConfig, network_lvl_1_num)
  MAP_PARAM(NocConfig, network_lvl_2_num)
  MAP_PARAM(NocConfig, network_lvl_3_num)
};
