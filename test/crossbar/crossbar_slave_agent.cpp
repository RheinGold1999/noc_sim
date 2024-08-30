#include "crossbar_slave_agent.h"
#include "crossbar_slave_driver.h"
#include "crossbar_slave_monitor.h"
#include "crossbar_slave_sequencer.h"

using namespace uvm;

crossbar_slave_agent::crossbar_slave_agent(const uvm_component_name& name)
: uvm_agent(name)
{}

void
crossbar_slave_agent::build_phase(uvm_phase& phase)
{
  uvm_agent::build_phase(phase);

  mon = crossbar_slave_monitor::type_id::create("mon", this);
  assert(mon);

  if (get_is_active() == UVM_ACTIVE) {
    sqr = crossbar_slave_sequencer::type_id::create("sqr", this);
    assert(sqr);
    drv = crossbar_slave_driver::type_id::create("drv", this);
    assert(drv);
  }

  if (!uvm_config_db<int>::get(this, "", "mst_id", slv_id)) {
    UVM_FATAL("NO_SLV_ID", "slv_id not set for: " + get_full_name());
  }
}

void
crossbar_slave_agent::connect_phase(uvm_phase& phase)
{
  if (get_is_active() == UVM_ACTIVE) {
    drv->seq_item_port.connect(sqr->seq_item_export);
    sqr->req_get_port.connect(mon->req_get_export);
  }
}

