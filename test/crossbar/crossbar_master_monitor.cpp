#include "crossbar_master_monitor.h"
#include "crossbar_if.h"

using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace uvm;

crossbar_master_monitor::crossbar_master_monitor(const uvm_component_name& name)
: uvm_monitor(name)
{}

void
crossbar_master_monitor::build_phase(uvm_phase& phase)
{
  uvm_monitor::build_phase(phase);

  if (!uvm_config_db<crossbar_if*>::get(this, "", "vif", vif)) {
    UVM_FATAL("NO_VIF", "vif is not set for: " + get_full_name());
  }

  if (!uvm_config_db<int>::get(this, "", "mst_id", mst_id)) {
    UVM_FATAL("NO_MST_ID", "mst_id is not set for: " + get_full_name());
  }
}

void
crossbar_master_monitor::connect_phase(uvm_phase& phase)
{
  assert(vif);
  req_get_port.bind(vif->req_get_export);
  rsp_get_port.bind(vif->rsp_get_export);
}

void
crossbar_master_monitor::run_phase(uvm_phase& phase)
{
  SC_FORK
    sc_spawn(std::bind(&crossbar_master_monitor::collect_req, this)),
    sc_spawn(std::bind(&crossbar_master_monitor::collect_rsp, this))
  SC_JOIN
}

void
crossbar_master_monitor::collect_req()
{
  crossbar_transfer req{"req"};
  while (true) {
    req_get_port->get(req);
    req_ap.write(req);
  }
}

void
crossbar_master_monitor::collect_rsp()
{
  crossbar_transfer rsp{"rsp"};
  while (true) {
    rsp_get_port->get(rsp);
    rsp_ap.write(rsp);
  }
}

