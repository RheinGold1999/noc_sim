#ifndef __CROSSBAR_SCOREBOARD_H__
#define __CROSSBAR_SCOREBOARD_H__

// #include <systemc>
#include <tlm>
#include <uvm>
#include <map>

#include "crossbar_transfer.h"

class AddrDecoder;

class crossbar_scoreboard : public uvm::uvm_scoreboard
{
  typedef tlm::tlm_analysis_fifo<crossbar_transfer> analysis_fifo_type;

public:
  std::vector<analysis_fifo_type*> mst_req_collected_fifos;
  std::vector<analysis_fifo_type*> mst_rsp_collected_fifos;

  std::vector<analysis_fifo_type*> slv_req_collected_fifos;
  std::vector<analysis_fifo_type*> slv_rsp_collected_fifos;
  
public:
  crossbar_scoreboard(const uvm::uvm_component_name& name);

  UVM_COMPONENT_UTILS(crossbar_scoreboard);

  void build_phase(uvm::uvm_phase& phase);
  // void connect_phase(uvm::uvm_phase& phase);
  void run_phase(uvm::uvm_phase& phase);
  void final_phase(uvm::uvm_phase& phase);

  void mst_req_collect_thread(int mst_id);
  void mst_rsp_collect_thread(int mst_id);
  void slv_req_collect_thread(int slv_id);
  void slv_rsp_collect_thread(int slv_id);

  void update_expected_mem(const crossbar_transfer& trans);
  void check_expected_mem(const crossbar_transfer& trans);

  int get_err_num();

private:
  AddrDecoder* m_addr_dec{nullptr};

  int num_mst;
  int num_slv;

  std::map<uint64_t, uint8_t> mem_expected;
  std::map<uint64_t, int> req_id_slv_id_map;
  std::map<uint64_t, int> rsp_id_mst_id_map;

  int num_reads;
  int num_writes;

  int num_errors;

  bool disabled;
};

#endif /* __CROSSBAR_SCOREBOARD_H__ */
