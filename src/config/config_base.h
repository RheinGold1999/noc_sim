#ifndef __CONFIG_BASE_H__
#define __CONFIG_BASE_H__

#include <string>
#include <map>

#include "config/parameter.h"

#define DECL_PARAM(type, name)  \
  static Parameter<type> name;

#define INIT_PARAM(cfg, type, name, val, desc) \
  Parameter<type> cfg::name = Parameter<type>(#name, val, desc);

#define MAP_PARAM(cfg, param) \
  {cfg::param.name(), &cfg::param},

struct ConfigBase
{
  typedef std::map<std::string, ParameterBase*> param_map_type;
};


#endif  /* __CONFIG_BASE_H__ */
