#ifndef __CROSSBAR_SLAVE_AGENT_H__
#define __CROSSBAR_SLAVE_AGENT_H__

#include <tlm>
#include <uvm>

class crossbar_slave_driver;
class crossbar_slave_sequencer;
class crossbar_slave_monitor;

class crossbar_slave_agent : public uvm::uvm_agent
{
public:
  crossbar_slave_driver* drv;
  crossbar_slave_sequencer* sqr;
  crossbar_slave_monitor* mon;

  crossbar_slave_agent(const uvm::uvm_component_name& name);

  UVM_COMPONENT_UTILS(crossbar_slave_agent);

  void build_phase(uvm::uvm_phase& phase);
  void connect_phase(uvm::uvm_phase& phase);

protected:
  int slv_id;
};

#endif /* __CROSSBAR_SLAVE_AGENT_H__ */
