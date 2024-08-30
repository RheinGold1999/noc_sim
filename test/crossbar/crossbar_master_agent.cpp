#include "crossbar_master_agent.h"
#include "crossbar_master_driver.h"
#include "crossbar_master_sequencer.h"
#include "crossbar_master_monitor.h"


using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace uvm;

crossbar_master_agent::crossbar_master_agent(const uvm_component_name& name)
: uvm_agent(name)
{

}

void
crossbar_master_agent::build_phase(uvm_phase& phase)
{
  uvm_agent::build_phase(phase);
  
  if (!uvm_config_db<int>::get(this, "", "mst_id", mst_id)) {
    UVM_FATAL("NO_MST_ID", "mst_id not set for: " + get_full_name());
  }

  mon = crossbar_master_monitor::type_id::create("mon", this);
  assert(mon);

  if (get_is_active() == UVM_ACTIVE) {
    sqr = crossbar_master_sequencer::type_id::create("sqr", this);
    assert(sqr);
    drv = crossbar_master_driver::type_id::create("drv", this);
    assert(drv);
  }
}

void
crossbar_master_agent::connect_phase(uvm_phase& phase)
{
  if (get_is_active() == UVM_ACTIVE) {
    drv->seq_item_port.connect(sqr->seq_item_export);
  }
}
