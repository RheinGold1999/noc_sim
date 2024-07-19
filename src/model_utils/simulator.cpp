#include "model_utils/simulator.h"
#include "model_utils/model_base.h"
#include "model_utils/top.h"
#include "model_utils/port.h"
#include "log/logger.h"
#include "config/global_config.h"

#include <list>

uint64_t Simulator::s_tick = 0;
bool Simulator::s_stop = false;

uint64_t
Simulator::curr_tick()
{
  return s_tick;
}

void
Simulator::set_stop()
{
  s_stop = true;
}

void
Simulator::run(uint64_t cyc)
{
  elaborate();
  simulate(cyc);
  finalize();
}

void
Simulator::simulate(uint64_t cyc)
{
  Top* top = Top::instance();
  while (true) {
    top->_cyc_phase_1();
    top->_cyc_phase_2();
    top->_cyc_phase_3();

    if ((cyc > 0 && s_tick >= cyc) || s_stop) {
      break;
    }

    s_tick++;
  }
}

void
Simulator::elaborate()
{
  check_ports_connection();
  update_logger_level();
}

void
Simulator::finalize()
{
  
}

void
Simulator::check_ports_connection()
{
  std::list<const ModelBase*> unbound_ports_list;

  for (auto mdl : ModelBase::walk_tree_node(Top::instance())) {
    if (    mdl->m_type == ModelBase::ModelType::PORT
         && !dynamic_cast<const PortBase*>(mdl)->is_bound() 
    ) {
      unbound_ports_list.emplace_back(mdl);
    }
  }

  if (!unbound_ports_list.empty()) {
    for (const auto p : unbound_ports_list) {
      _ERROR("The port {} is unbound", p->full_name());
    }
    assert(false);
  }
}

void
Simulator::update_logger_level()
{
  Top::logger->set_level((int)GlobalConfig::top_dbg_lvl);
  _INFO("Top logger level is: {}", (int)Top::logger->m_spdlogger->level());
}

