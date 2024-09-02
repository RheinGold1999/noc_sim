#ifndef __CROSSBAR_SLAVE_DRIVER_H__
#define __CROSSBAR_SLAVE_DRIVER_H__

#include <uvm>
#include <systemc>
#include <tlm>
#include "tlm_utils/simple_target_socket.h"

#include "crossbar_transfer.h"

class crossbar_if;

class crossbar_slave_driver : public uvm::uvm_driver<crossbar_transfer>
{
public:
  tlm_utils::simple_target_socket<crossbar_slave_driver> target_socket;

public:
  crossbar_slave_driver(const uvm::uvm_component_name& name);

  UVM_COMPONENT_UTILS(crossbar_slave_driver);

  void build_phase(uvm::uvm_phase& phase);
  void connect_phase(uvm::uvm_phase& phase);
  void run_phase(uvm::uvm_phase& phase);

  void drive_rsp();

  //----------------------------------------------------------------------------
  // TLM2.0 Interface
  //----------------------------------------------------------------------------
  tlm::tlm_sync_enum nb_transport_fw(
    tlm::tlm_generic_payload& gp,
    tlm::tlm_phase& phase,
    sc_core::sc_time& delay
  );

private:
  crossbar_if* vif{nullptr};

  int slv_id;

  crossbar_transfer req{"req"};
  crossbar_transfer rsp{"rsp"};

  sc_core::sc_event rsp_end_ev{"rsp_end_ev"};
};


#endif /* __CROSSBAR_SLAVE_DRIVER_H__ */
