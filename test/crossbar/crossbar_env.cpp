#include "crossbar_env.h"
#include "crossbar_master_agent.h"
#include "crossbar_master_monitor.h"
#include "crossbar_slave_agent.h"
#include "crossbar_slave_monitor.h"
#include "crossbar_if.h"
#include "crossbar_scoreboard.h"

using namespace uvm;
using namespace std;

crossbar_env::crossbar_env(const uvm::uvm_component_name& name)
: uvm_env(name)
, num_mst(0)
, num_slv(0)
{}

void
crossbar_env::build_phase(uvm_phase& phase)
{
  uvm_env::build_phase(phase);

  if (!uvm_config_db<int>::get(this, "", "num_mst", num_mst)) {
    UVM_FATAL("NO_NUM_MST", "num_mst must be set for: " + get_full_name());
  }
  if (!uvm_config_db<int>::get(this, "", "num_slv", num_slv)) {
    UVM_FATAL("NO_NUM_SLV", "num_slv must be set for: " + get_full_name());
  }

  uvm_config_db<int>::set(this, "*", "num_mst", num_mst);
  uvm_config_db<int>::set(this, "*", "num_slv", num_slv);

  master_vifs.resize(num_mst);
  slave_vifs.resize(num_slv);
  
  for (int i = 0; i < num_mst; ++i) {
    char mst_name[20];
    sprintf(mst_name, "masters[%d]", i);
    crossbar_master_agent* mst = 
      crossbar_master_agent::type_id::create(mst_name, this);
    masters.emplace_back(mst);

    uvm_config_db<int>::set(this, mst_name, "*.mst_id", i);
  
    char mst_vif_name[20];
    sprintf(mst_vif_name, "master_vifs[%d]", i);
    if (!uvm_config_db<crossbar_if*>::get(this, "", mst_vif_name, master_vifs[i])) {
      UVM_FATAL("NO_VIF", "vif must be set for: " + get_full_name() + "." + mst_vif_name);
    }
    uvm_config_db<crossbar_if*>::set(this, mst_name, "*.vif", master_vifs[i]);
  }

  for (int i = 0; i < num_slv; ++i) {
    char slv_name[20];
    sprintf(slv_name, "slaves[%d]", i);
    crossbar_slave_agent* slv = 
      crossbar_slave_agent::type_id::create(slv_name, this);
    slaves.emplace_back(slv);

    uvm_config_db<int>::set(this, slv_name, "*.slv_id", i);
    
    char slv_vif_name[20];
    sprintf(slv_vif_name, "slave_vifs[%d]", i);
    if (!uvm_config_db<crossbar_if*>::get(this, "", slv_vif_name, slave_vifs[i])) {
      UVM_FATAL("NO_VIF", "vif must be set for: " + get_full_name() + "." + slv_vif_name);
    }
    uvm_config_db<crossbar_if*>::set(this, slv_name, "*.vif", slave_vifs[i]);
  }
  
  sbd = crossbar_scoreboard::type_id::create("sbd", this);
  assert(sbd);
}

void
crossbar_env::connect_phase(uvm_phase& phase)
{
  for (int i = 0; i < num_mst; ++i) {
    masters[i]->mon->req_ap.connect(sbd->mst_req_collected_fifos[i]);
    masters[i]->mon->rsp_ap.connect(sbd->mst_rsp_collected_fifos[i]);
  }
  for (int i = 0; i < num_slv; ++i) {
    slaves[i]->mon->req_ap.connect(sbd->slv_req_collected_fifos[i]);
    slaves[i]->mon->rsp_ap.connect(sbd->slv_rsp_collected_fifos[i]);
  }
}

void
crossbar_env::run_phase(uvm_phase& phase)
{

}
