#ifndef __CROSSBAR_MASTER_MONITOR_H__
#define __CROSSBAR_MASTER_MONITOR_H__

#include <uvm>

#include "crossbar_transfer.h"

class crossbar_if;

class crossbar_master_monitor : public uvm::uvm_monitor
{
public:
  // connected to vif
  uvm::uvm_blocking_get_port<crossbar_transfer> req_get_port{"req_get_port"};
  uvm::uvm_blocking_get_port<crossbar_transfer> rsp_get_port{"rsp_get_port"};

  // connected to scoreboard
  uvm::uvm_analysis_port<crossbar_transfer> req_ap{"req_ap"};
  uvm::uvm_analysis_port<crossbar_transfer> rsp_ap{"req_ap"};

  crossbar_master_monitor(const uvm::uvm_component_name& name);

  UVM_COMPONENT_UTILS(crossbar_master_monitor);

  void build_phase(uvm::uvm_phase& phase);
  void connect_phase(uvm::uvm_phase& phase);
  void run_phase(uvm::uvm_phase& phase);

  void collect_req();
  void collect_rsp();

private:
  crossbar_if* vif{nullptr};
  int mst_id;
};

#endif /* __CROSSBAR_MASTER_MONITOR_H__ */
