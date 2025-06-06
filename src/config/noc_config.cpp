#include "config/noc_config.h"

INIT_PARAM(NocConfig, int, network_lvl_0_num, 10, "network level 0 number (should contain NodeRouters and BridgeRouters in case of the Ring structure)")
INIT_PARAM(NocConfig, int, network_lvl_1_num, 2, "network level 1 number (should contain NodeRouters and BridgeRouters in case of the Ring structure)")
INIT_PARAM(NocConfig, int, network_lvl_2_num, 1, "network level 2 number (should contain NodeRouters and BridgeRouters in case of the Ring structure)")
INIT_PARAM(NocConfig, int, network_lvl_3_num, 1, "network level 3 number (should contain NodeRouters and BridgeRouters in case of the Ring structure)")
INIT_PARAM(NocConfig, int, network_lvl_0_bridge_num, 2, "network level 0 bridge number")
INIT_PARAM(NocConfig, int, network_lvl_1_bridge_num, 1, "network level 1 bridge number")
INIT_PARAM(NocConfig, int, ring_width, 2, "ring width (should be even)")
INIT_PARAM(NocConfig, int, node_inj_que_depth, 8, "node injection queue depth")
INIT_PARAM(NocConfig, int, node_eje_que_depth, 8, "node ejection queue depth")
INIT_PARAM(NocConfig, int, bridge_inj_que_depth, 16, "bridge injection queue depth")
INIT_PARAM(NocConfig, int, bridge_eje_que_depth, 16, "bridge ejection queue depth")
INIT_PARAM(NocConfig, double, node_inj_rate, 0.5, "node injection rate (shouble be between (0. 1.0))")

std::map<std::string, ParameterBase*> NocConfig::param_map = {
  MAP_PARAM(NocConfig, network_lvl_0_num)
  MAP_PARAM(NocConfig, network_lvl_1_num)
  MAP_PARAM(NocConfig, network_lvl_2_num)
  MAP_PARAM(NocConfig, network_lvl_3_num)
  MAP_PARAM(NocConfig, network_lvl_0_bridge_num)
  MAP_PARAM(NocConfig, network_lvl_1_bridge_num)
  MAP_PARAM(NocConfig, ring_width)
  MAP_PARAM(NocConfig, node_inj_que_depth)
  MAP_PARAM(NocConfig, node_eje_que_depth)
  MAP_PARAM(NocConfig, bridge_inj_que_depth)
  MAP_PARAM(NocConfig, bridge_eje_que_depth)
  MAP_PARAM(NocConfig, node_inj_rate)
};
