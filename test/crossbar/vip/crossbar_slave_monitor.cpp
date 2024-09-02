#include "crossbar_slave_monitor.h"
#include "crossbar_if.h"

using namespace uvm;
using namespace sc_core;

crossbar_slave_monitor::crossbar_slave_monitor(const uvm_component_name& name)
: uvm_monitor(name)
{
  req_get_export.bind(req_fifo);
}

void
crossbar_slave_monitor::build_phase(uvm_phase& phase)
{
  uvm_monitor::build_phase(phase);

  if (!uvm_config_db<crossbar_if*>::get(this, "", "vif", vif)) {
    UVM_FATAL("NO_VIF", "vif is not set for: " + get_full_name());
  }

  if (!uvm_config_db<int>::get(this, "", "slv_id", slv_id)) {
    UVM_FATAL("NO_SLV_ID", "slv_id is not set for: " + get_full_name());
  }
}

void
crossbar_slave_monitor::connect_phase(uvm_phase& phase)
{
  if (vif == nullptr) {
    UVM_FATAL("VIF_IS_NULL", "vif is not set for: "
      + get_full_name());
  }
  req_get_port.bind(vif->req_get_export);
  rsp_get_port.bind(vif->rsp_get_export);
}

void
crossbar_slave_monitor::run_phase(uvm_phase& phase)
{
  SC_FORK
    sc_spawn(sc_bind(&crossbar_slave_monitor::collect_req, this)),
    sc_spawn(sc_bind(&crossbar_slave_monitor::collect_rsp, this))
  SC_JOIN
}

void
crossbar_slave_monitor::collect_req()
{
  crossbar_transfer req{"req"};
  while (true) {
    req_get_port.get(req);
    req_ap.write(req);
    req_fifo.put(req);
  }
}

void
crossbar_slave_monitor::collect_rsp()
{
  crossbar_transfer rsp{"rsp"};
  while (true) {
    rsp_get_port.get(rsp);
    rsp_ap.write(rsp);
  }
}

