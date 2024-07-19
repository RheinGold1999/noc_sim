#ifndef __CONFIG_MANAGER_H__
#define __CONFIG_MANAGER_H__

#include <map>

#include "config/parameter.h"

struct ConfigManager
{
  typedef std::map<std::string, ParameterBase*> param_map_type;

  static std::map<const std::string, param_map_type*> cfg_map;

  static void parse_args(int argc, char** argv);

  static void print_args();

  static void update_param(
    param_map_type& param_map,
    const std::string& opt,
    const std::string& val
  );

  template<typename T>
  static void map_check_and_set(
    param_map_type& param_map,
    const std::string& param_name,
    T param_val
  );

};


#endif /* __CONFIG_MANAGER_H__ */
