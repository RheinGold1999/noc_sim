#ifndef __CROSSBAR_MASTER_DRIVER_H__
#define __CROSSBAR_MASTER_DRIVER_H__

#include <systemc>
#include <tlm>
#include <uvm>
#include "tlm_utils/simple_initiator_socket.h"

#include "crossbar_transfer.h"

class crossbar_if;

class crossbar_master_driver : public uvm::uvm_driver<crossbar_transfer>
{
public:
  tlm_utils::simple_initiator_socket<crossbar_master_driver> initiator_socket;

public:
  crossbar_master_driver(const uvm::uvm_component_name& name);

  UVM_COMPONENT_UTILS(crossbar_master_driver);

  void build_phase(uvm::uvm_phase& phase);
  void connect_phase(uvm::uvm_phase& phase);
  void run_phase(uvm::uvm_phase& phase);

  void drive_req();

  //----------------------------------------------------------------------------
  // TLM2.0 Interface
  //----------------------------------------------------------------------------
  tlm::tlm_sync_enum nb_transport_bw(
    tlm::tlm_generic_payload& gp,
    tlm::tlm_phase& phase,
    sc_core::sc_time& delay
  );
 
private:
  crossbar_if* vif{nullptr};

  sc_core::sc_event req_end_ev{"req_end_ev"};
  sc_core::sc_event rsp_start_ev{"rsp_start_ev"};

  int mst_id;

  crossbar_transfer req{"req"};
  crossbar_transfer rsp{"rsp"};

  std::map<uint64_t, crossbar_transfer> m_trans_id_inflight_req_map;
};

#endif /* __CROSSBAR_MASTER_DRIVER_H__ */
