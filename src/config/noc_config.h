#ifndef __NOC_CONFIG_H__
#define __NOC_CONFIG_H__

#include "config/config_base.h"

struct NocConfig
  : ConfigBase
{
  static param_map_type param_map;

  DECL_PARAM(int, network_lvl_0_num)
  DECL_PARAM(int, network_lvl_1_num)
  DECL_PARAM(int, network_lvl_2_num)
  DECL_PARAM(int, network_lvl_3_num)

  DECL_PARAM(int, ring_width)

  DECL_PARAM(int, node_inj_que_depth)
  DECL_PARAM(int, node_eje_que_depth)
  DECL_PARAM(int, bridge_inj_que_depth)
  DECL_PARAM(int, bridge_eje_que_depth)

};

#endif  /* __NOC_CONFIG_H__ */
