#ifndef __NOC_CONFIG_H__
#define __NOC_CONFIG_H__

#include "config/config_base.h"

struct NocConfig
  : ConfigBase
{
  static param_map_type param_map;

  DECL_PARAM(int, hellow)
  DECL_PARAM(int, world)
  DECL_PARAM(int, are)
  DECL_PARAM(double, you)
  DECL_PARAM(double, ok)
};

#endif  /* __NOC_CONFIG_H__ */
