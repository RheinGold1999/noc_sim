#include "crossbar_slave_driver.h"
#include "crossbar_if.h"
#include "crossbar/tlm_gp_mm.h"

using namespace uvm;
using namespace tlm;
using namespace sc_core;

crossbar_slave_driver::crossbar_slave_driver(const uvm_component_name& name)
: uvm_driver<crossbar_transfer>(name)
{
  target_socket.register_nb_transport_fw(this, &crossbar_slave_driver::nb_transport_fw);
}

void
crossbar_slave_driver::build_phase(uvm_phase& phase)
{
  uvm_driver<crossbar_transfer>::build_phase(phase);

  if (!uvm_config_db<crossbar_if*>::get(this, "", "vif", vif)) {
    UVM_FATAL("NO_VIF", "virtual interface must be set for: "
      + get_full_name() + ".vif");
  }

  if (!uvm_config_db<int>::get(this, "", "slv_id", slv_id)) {
    UVM_FATAL("NO_SLV_ID", "Slave ID not found for: "
      + get_full_name() + ".vif");
  }
}

void
crossbar_slave_driver::connect_phase(uvm_phase& phase)
{
  if (vif == nullptr) {
    UVM_FATAL("VIF_IS_NULL", "vif is not set for: "
      + get_full_name());
  }
  vif->initiator_socket.bind(target_socket);
}

void
crossbar_slave_driver::run_phase(uvm_phase& phase)
{
  SC_FORK
    sc_spawn(sc_bind(&crossbar_slave_driver::drive_rsp, this))
  SC_JOIN
}

void
crossbar_slave_driver::drive_rsp()
{
  while (true) {
    this->seq_item_port.get_next_item(rsp);
    tlm_generic_payload* gp = tlm_gp_mm::instance().allocate();
    rsp.copy_to_gp(*gp);

    tlm_phase phase = BEGIN_RESP;
    sc_time delay = SC_ZERO_TIME;
    target_socket->nb_transport_bw(*gp, phase, delay);
    if (phase == BEGIN_RESP) {
      wait(rsp_end_ev);
    }
    gp->release();
    this->seq_item_port.item_done();
  }
}

tlm_sync_enum
crossbar_slave_driver::nb_transport_fw(
  tlm_generic_payload& gp,
  tlm_phase& phase,
  sc_time& delay
)
{
  // The requ
  tlm_sync_enum ret_status = TLM_ACCEPTED;
  if (phase == BEGIN_REQ) {
    phase = END_REQ;
    ret_status = TLM_UPDATED;
  } else if (phase == END_RESP) {
    rsp_end_ev.notify();
    ret_status = TLM_COMPLETED;
  } else {
    UVM_FATAL("WRONG_TLM_PHASE", phase.get_name());
  }

  return ret_status;
}

