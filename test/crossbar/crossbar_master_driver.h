#ifndef __CROSSBAR_MASTER_DRIVER_H__
#define __CROSSBAR_MASTER_DRIVER_H__

#include <systemc>
#include <tlm>
#include <uvm>
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "crossbar_transfer.h"

class crossbar_if;

class crossbar_master_driver : public uvm::uvm_driver<crossbar_transfer>
{
public:
  tlm_utils::simple_initiator_socket<crossbar_master_driver> initiator_socket;

  crossbar_if* vif{nullptr};

public:
  UVM_COMPONENT_UTILS(crossbar_master_driver);

  crossbar_master_driver(const uvm::uvm_component_name& name);

  void build_phase(uvm::uvm_phase& phase);
  void connect_phase(uvm::uvm_phase& phase);
  void run_phase(uvm::uvm_phase& phase);

  void get_req();
  void rcv_rsp();

  //----------------------------------------------------------------------------
  // TLM2.0 Interface
  //----------------------------------------------------------------------------
  tlm::tlm_sync_enum nb_transport_bw(
    tlm::tlm_generic_payload& gp,
    tlm::tlm_phase& phase,
    sc_core::sc_time& delay
  );
 
private:
  sc_core::sc_event_queue req_end_ev_que;
  sc_core::sc_event_queue rsp_start_ev_que;

  int mst_id;

  crossbar_transfer req{"req"};
  crossbar_transfer rsp{"rsp"};

  std::map<tlm::tlm_generic_payload*, >
};

#endif /* __CROSSBAR_MASTER_DRIVER_H__ */
