#ifndef __CROSSBAR_SLAVE_MONITOR_H__
#define __CROSSBAR_SLAVE_MONITOR_H__

#include <uvm>

#include "crossbar_transfer.h"

class crossbar_if;

class crossbar_slave_monitor : public uvm::uvm_monitor
{
public:
  // connected to vif
  uvm::uvm_blocking_get_port<crossbar_transfer> req_get_port{"req_get_port"};
  uvm::uvm_blocking_get_port<crossbar_transfer> rsp_get_port{"rsp_get_port"};

  // connected to scoreboard
  uvm::uvm_analysis_port<crossbar_transfer> req_ap{"req_ap"};
  uvm::uvm_analysis_port<crossbar_transfer> rsp_ap{"rsp_ap"};

  // connected to slave sequencer
  uvm::uvm_blocking_get_export<crossbar_transfer> req_get_export;

public:
  crossbar_slave_monitor(const uvm::uvm_component_name& name);

  UVM_COMPONENT_UTILS(crossbar_slave_monitor);

  void build_phase(uvm::uvm_phase& phase);
  void connect_phase(uvm::uvm_phase& phase);
  void run_phase(uvm::uvm_phase& phase);

  void collect_req();
  void collect_rsp();

  // req_peek_imp interface
  crossbar_transfer peek();

private:
  crossbar_if* vif{nullptr};
  int slv_id;
  tlm::tlm_fifo<crossbar_transfer> req_fifo{"req_fifo", 128};
};

#endif /* __CROSSBAR_SLAVE_MONITOR_H__ */
