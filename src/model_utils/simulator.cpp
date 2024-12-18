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
  _INFO("elaboration done...");
  simulate(cyc);
  _INFO("simulation done...");
  finalize();
  _INFO("finalization done...")
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
  Top* top = Top::instance();
  top->_elaborate();
}

void
Simulator::finalize()
{
  Top* top = Top::instance();
  top->_finalize();
}

