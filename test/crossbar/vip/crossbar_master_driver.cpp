#include "crossbar_master_driver.h"
#include "crossbar_master_monitor.h"
#include "crossbar_master_sequencer.h"
#include "crossbar_if.h"
#include "crossbar/tlm_gp_mm.h"

using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace uvm;

crossbar_master_driver::crossbar_master_driver(const uvm_component_name& name)
: uvm_driver<crossbar_transfer>(name)
{
  initiator_socket.register_nb_transport_bw(this, &crossbar_master_driver::nb_transport_bw);
}

void
crossbar_master_driver::build_phase(uvm_phase& phase)
{
  uvm_driver<crossbar_transfer>::build_phase(phase);

  if (!uvm_config_db<crossbar_if*>::get(this, "", "vif", vif)) {
    UVM_FATAL("NO_VIF", "virtual interface must be set for: "
      + get_full_name() + ".vif");
  }

  if (!uvm_config_db<int>::get(this, "", "mst_id", mst_id)) {
    UVM_FATAL("NO_MST_ID", "Master ID not found for: "
      + get_full_name() + ".vif");
  }
}

void
crossbar_master_driver::connect_phase(uvm_phase& phase)
{
  if (vif == nullptr) {
    UVM_FATAL("VIF_IS_NULL", "vif is not set for: "
      + get_full_name());
  }
  initiator_socket.bind(vif->target_socket);
}

void
crossbar_master_driver::run_phase(uvm_phase& phase)
{
  SC_FORK
    sc_spawn(sc_bind(&crossbar_master_driver::drive_req, this))
  SC_JOIN
}

void
crossbar_master_driver::drive_req()
{
  while (true) {
    this->seq_item_port.get_next_item(req);
    tlm_generic_payload* gp = tlm_gp_mm::instance().allocate();
    req.copy_to_gp(*gp);
    m_trans_id_inflight_req_map[req.id] = req;

    tlm_phase phase = BEGIN_REQ;
    sc_time delay = SC_ZERO_TIME;
    initiator_socket->nb_transport_fw(*gp, phase, delay);
    if (phase == BEGIN_REQ) {
      wait(req_end_ev);
    }

    this->seq_item_port.item_done();
  }
}

tlm_sync_enum
crossbar_master_driver::nb_transport_bw(
  tlm_generic_payload& gp,
  tlm_phase& phase,
  sc_time& delay
)
{
  tlm_sync_enum ret_status = TLM_ACCEPTED;
  if (phase == END_REQ) {
    req_end_ev.notify();
  } else if (phase == BEGIN_RESP) {
    phase = END_RESP;
    ret_status = TLM_COMPLETED;
    rsp.init_by_gp(gp);
    auto it = m_trans_id_inflight_req_map.find(rsp.id);
    sc_assert(it != m_trans_id_inflight_req_map.end());
    rsp.set_id_info(it->second);
    m_trans_id_inflight_req_map.erase(it);
    gp.release();
    this->seq_item_port.put_response(rsp);
    // rsp_start_ev.notify();
  } else {
    UVM_FATAL("WRONG_TLM_PHASE", phase.get_name());
  }

  return ret_status;
}
