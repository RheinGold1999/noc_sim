#include <regex>

#include "config/config_manager.h"
#include "config/global_config.h"
#include "config/noc_config.h"
#include "log/logger.h"

std::map<const std::string, ConfigManager::param_map_type*>
ConfigManager::cfg_map = {
  {".glb.", &GlobalConfig::param_map},
  {".noc.", &NocConfig::param_map},
};

void 
ConfigManager::parse_args(int argc, char** argv)
{
  // parse command line
  for (int i = 1; i < argc - 1; i += 2) {
    // DEMO(argv[i]);
    param_map_type* param_map_ptr = nullptr;
    std::string opt = argv[i];
    std::string val = argv[i + 1];
    if ( opt.find("-config.") != std::string::npos ||
         opt.find("-cfg.") != std::string::npos ) {
      bool is_found_in_cfg_map = false;
      for (auto pair : cfg_map) {
        if (opt.find(pair.first) != std::string::npos) {
          param_map_ptr = pair.second;
          is_found_in_cfg_map = true;
          break;
        }
      }
      if (!is_found_in_cfg_map) {
        _ERROR("config parameter `{}` does NOT exist!!!", opt);
        abort();
      }
    }
    ConfigManager::update_param(*param_map_ptr, opt, val);
  }
}

void
ConfigManager::update_param(
  param_map_type& param_map,
  const std::string& opt,
  const std::string& val
)
{
  static std::regex name_pat("-{1,2}(?:cfg|config)\\S*\\.(\\w+)");
  static std::smatch name_matchs;
  bool name_matched = std::regex_match(opt, name_matchs, name_pat);
  _ASSERT(name_matched);
  std::string param_name = name_matchs[1];

  static std::regex type_int_dec_pat("(\\d+)");
  static std::regex type_int_hex_pat("(0[xX][0-9a-fA-F]+)");
  static std::regex type_double_pat("(\\d+\\.\\d+)");
  static std::smatch val_matchs;

  if (param_map.count(param_name) == 1) {
    if (std::regex_match(val, val_matchs, type_int_dec_pat)) {
      int param_val = std::stoi(val);
      ConfigManager::map_check_and_set<int>(param_map, param_name, param_val);
    } else if (std::regex_match(val, val_matchs, type_int_hex_pat)) {
      int param_val = std::stoi(val, nullptr, 16);
      ConfigManager::map_check_and_set<int>(param_map, param_name, param_val);
    } else if (std::regex_match(val, val_matchs, type_double_pat)) {
      double param_val = std::stod(val);
      ConfigManager::map_check_and_set<double>(param_map, param_name, param_val);
    } else {
      // std::cout << "parameter `" << param_name << "` type doesn't found!!!" << std::endl;
      _ERROR("parameter `{}` type does NOT found!!!", param_name);
      abort();
    }
  } else {
    // std::cout << "parameter `" << param_name << "` doesn't exist!!!" << std::endl;
    _ERROR("parameter `{}` does NOT exist!!!", param_name);
    abort();
  }
}

template<typename T>
void
ConfigManager::map_check_and_set(
    param_map_type& param_map,
    const std::string& param_name,
    T param_val
)
{
  if (param_map.count(param_name) != 1) {
    // std::cout << "parameter `" << param_name << "` doesn't exist!!!" << std::endl;
    _ERROR("parameter `{}` does NOT exist!!!", param_name);
    abort();
  }
  if (param_map[param_name]->m_type != typeid(T)) {
    // std::cout << "parameter `" << param_name << "` type doesn't match!!!" << std::endl;
    _ERROR("parameter `{}` type does NOT match!!!", param_name);
    abort();
  }
  reinterpret_cast<Parameter<T>*>(param_map[param_name])->set_val(param_val);
}

void
ConfigManager::print_args()
{
  // _INFO("-----------------------------------------------------------------------");
  // _INFO("                           PARAMETERS                                  ")
  // _INFO("-----------------------------------------------------------------------");
  // for (auto cfg_pair : cfg_map) {
  //   for (auto param_pair : *cfg_pair.second) {
  //     _INFO("{}: {}", param_pair.first, *param_pair.second);
  //   }
  // }
}

