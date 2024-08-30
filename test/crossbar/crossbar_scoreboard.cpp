#include "crossbar_scoreboard.h"
#include "crossbar/addr_decoder.h"

#include <list>

using namespace std;
using namespace uvm;
using namespace tlm;
using namespace sc_core;

crossbar_scoreboard::crossbar_scoreboard(const uvm_component_name& name)
: uvm_scoreboard(name)
, num_mst(0)
, num_slv(0)
, num_reads(0)
, num_writes(0)
, num_errors(0)
, disabled(false)
{}

void
crossbar_scoreboard::build_phase(uvm_phase& phase)
{
  if (!uvm_config_db<int>::get(this, "", "num_mst", num_mst)) {
    UVM_FATAL("NUM_MST_NOT_SET", "num_mst not set for: "
      + get_full_name());
  }
  sc_assert(num_mst > 0);

  if (!uvm_config_db<int>::get(this, "", "num_slv", num_slv)) {
    UVM_FATAL("NUM_SLV_NOT_SET", "num_slv not set for: "
      + get_full_name());
  }
  sc_assert(num_slv > 0);

  // build fifo arrays based on num_mst and num_slv
  for (int i = 0; i < num_mst; ++i) {
    char fifo_name[50];
    sprintf(fifo_name, "mst_req_collected_fifo_[%d]", i);
    mst_req_collected_fifos.emplace_back(analysis_fifo_type{fifo_name});

    sprintf(fifo_name, "mst_rsp_collected_fifo_[%d]", i);
    mst_rsp_collected_fifos.emplace_back(analysis_fifo_type{fifo_name});
  }

  for (int i = 0; i < num_slv; ++i) {
    char fifo_name[50];
    sprintf(fifo_name, "slv_req_collected_fifo_[%d]", i);
    slv_req_collected_fifos.emplace_back(analysis_fifo_type{fifo_name});

    sprintf(fifo_name, "slv_rsp_collected_fifo_[%d]", i);
    slv_rsp_collected_fifos.emplace_back(analysis_fifo_type{fifo_name});
  }
}

void
crossbar_scoreboard::run_phase(uvm_phase& phase)
{
  /** SC_FORK & SC_JOIN cannot be employed directly in this case, 
   *  so we need to refer to their implementation in `sc_join.h`
   * 
   *  #define SC_FORK \
   *  { \
   *      sc_core::sc_process_handle forkees[] = {

   *  #define SC_JOIN \
   *      }; \
   *      sc_core::sc_join           join; \
   *      for ( unsigned int i = 0; \
   *          i < sizeof(forkees)/sizeof(sc_core::sc_process_handle); \
   *          i++ ) \
   *          join.add_process(forkees[i]); \
   *      join.wait(); \
   *  }
   */

  std::list<sc_process_handle> forkees;

  for (int i = 0; i < num_mst; ++i) {
    forkees.emplace_back(
      sc_spawn(sc_bind(&crossbar_scoreboard::mst_req_collect_thread, this, i))
    );
    forkees.emplace_back(
      sc_spawn(sc_bind(&crossbar_scoreboard::mst_rsp_collect_thread, this, i))
    );
  }
  for (int i = 0; i < num_slv; ++i) {
    forkees.emplace_back(
      sc_spawn(sc_bind(&crossbar_scoreboard::slv_req_collect_thread, this, i))
    );
    forkees.emplace_back(
      sc_spawn(sc_bind(&crossbar_scoreboard::slv_rsp_collect_thread, this, i))
    );
  }

  sc_join join;
  for (auto& proc : forkees) {
    join.add_process(proc);
  }
  join.wait();

}

void
crossbar_scoreboard::mst_req_collect_thread(int mst_id)
{
  crossbar_transfer req{"req"};

  while (true) {
    req = mst_req_collected_fifos[mst_id].get();
    AddrMapRule dec_res = g_default_addr_decoder.get_matched_rule(req.addr);
    req_id_slv_id_map.insert(make_pair(req.id, dec_res.id));
    rsp_id_mst_id_map.insert(make_pair(req.id, mst_id));
  }
}

void
crossbar_scoreboard::mst_rsp_collect_thread(int mst_id)
{
  crossbar_transfer rsp{"rsp"};

  while (true) {
    rsp = mst_rsp_collected_fifos[mst_id].get();
    auto it = rsp_id_mst_id_map.find(rsp.id);
    if (it != rsp_id_mst_id_map.end()) {
      if (mst_id != it->second) {
        UVM_ERROR("RSP_TO_WRONG_MST", "atual mst_id: "
          + to_string(mst_id) + ", expected mst_id: "
          + to_string(it->second));
        num_errors++;
      }
      rsp_id_mst_id_map.erase(it);
      if (rsp.cmd == READ) {
        check_expected_mem(rsp);
      }
    } else {
       UVM_ERROR("RSP_ID", "does NOT exist in rsp_id_mst_id_map: "
        + rsp.id);
      num_errors++;     
    }
  }
}

void
crossbar_scoreboard::check_expected_mem(const crossbar_transfer& trans)
{
  for (int i = 0; i < trans.size(); ++i) {
    if (mem_expected.find(trans.addr + i) == mem_expected.end()) {
      UVM_ERROR("READ_DATA_DONT_EXIST",
        "addr: "+ to_string(trans.addr + i));
    } else if (trans.data[i] != mem_expected[trans.addr + i]) {
      UVM_ERROR("READ_DATA_MISMATCH", 
        "addr: " + to_string(trans.addr + i)
        + ", actual data: " + to_string(trans.data[i])
        + ", expected data: " + to_string(mem_expected[trans.addr + i]));
      num_errors++;
    }
  }
}

void
crossbar_scoreboard::slv_req_collect_thread(int slv_id)
{
  crossbar_transfer req{"req"};

  while (true) {
    req = slv_req_collected_fifos[slv_id].get();
    auto it = req_id_slv_id_map.find(req.id);
    if (it != req_id_slv_id_map.end()) {
      if (slv_id != it->second) {
        UVM_ERROR("REQ_TO_WRONG_SLV", "atual slv_id: "
          + to_string(slv_id) + ", expected slv_id: "
          + to_string(it->second));
        num_errors++;
      }
      if (req.cmd == WRITE) {
        update_expected_mem(req);
      }
    } else {
      UVM_ERROR("REQ_ID", "does NOT exist in req_id_slv_id_map: "
        + req.id);
      num_errors++;
    }
  }
}

void
crossbar_scoreboard::update_expected_mem(const crossbar_transfer& trans)
{
  for (int i = 0; i < trans.size(); ++i) {
    mem_expected[trans.addr + i] = trans.data[i];
  }
}

void
crossbar_scoreboard::slv_rsp_collect_thread(int slv_id)
{
  crossbar_transfer rsp{"rsp"};

  while (true) {
    rsp = slv_rsp_collected_fifos[slv_id].get();
    if (rsp.cmd == READ) {
      for (int i = 0; i < rsp.size(); ++i) {
        if (mem_expected.find(rsp.addr + i) == mem_expected.end()) {
          mem_expected[rsp.addr + i] = rsp.data[i];
        }
      }
    }
  }
}
