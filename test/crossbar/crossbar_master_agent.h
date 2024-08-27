#ifndef __CROSSBAR_MASTER_AGENT_H__
#define __CROSSBAR_MASTER_AGENT_H__

#include <tlm>
#include <uvm>

class crossbar_master_driver;
class crossbar_master_monitor;
class crossbar_master_sequencer;

class crossbar_master_agent : public uvm::uvm_agent
{
public:
  crossbar_master_driver* drv;
  crossbar_master_sequencer* sqr;
  crossbar_master_monitor* mon;

  UVM_COMPONENT_UTILS(crossbar_master_agent);

  crossbar_master_agent(const uvm::uvm_component_name& name);

  void build_phase(uvm::uvm_phase& phase);
  void connect_phase(uvm::uvm_phase& phase);

protected:
  int mst_id;
};

#endif /* __CROSSBAR_MASTER_AGENT_H__ */
