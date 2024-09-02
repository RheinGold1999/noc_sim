#ifndef __CROSSBAR_ENV_H__
#define __CROSSBAR_ENV_H__

#include <systemc>
#include <uvm>

class crossbar_master_agent;
class crossbar_slave_agent;
class crossbar_if;
class crossbar_scoreboard;

class crossbar_env : public uvm::uvm_env
{
public:
  std::vector<crossbar_master_agent*> masters;
  std::vector<crossbar_if*> master_vifs;
  std::vector<crossbar_slave_agent*> slaves;
  std::vector<crossbar_if*> slave_vifs;

  crossbar_scoreboard* sbd;

  crossbar_env(const uvm::uvm_component_name& name);

  UVM_COMPONENT_UTILS(crossbar_env);

  void build_phase(uvm::uvm_phase& phase);
  void connect_phase(uvm::uvm_phase& phase);
  void run_phase(uvm::uvm_phase& phase);

protected:
  int num_mst;
  int num_slv;
};

#endif /* __CROSSBAR_ENV_H__ */
