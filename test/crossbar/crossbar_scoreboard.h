#ifndef __CROSSBAR_SCOREBOARD_H__
#define __CROSSBAR_SCOREBOARD_H__

// #include <systemc>
#include <tlm>
#include <uvm>
#include <map>

#include "crossbar_transfer.h"

class crossbar_scoreboard : public uvm::uvm_scoreboard
{
  typedef tlm::tlm_analysis_fifo<crossbar_transfer> analysis_fifo_type;

public:
  std::vector<analysis_fifo_type*> mst_req_collected_fifo;
  std::vector<analysis_fifo_type*> mst_rsp_collected_fifo;

  std::vector<analysis_fifo_type*> slv_req_collected_fifo;
  std::vector<analysis_fifo_type*> slv_rsp_collected_fifo;
  
public:
  crossbar_scoreboard(const uvm::uvm_component_name& name);

  UVM_COMPONENT_UTILS(crossbar_scoreboard);

  void build_phase(uvm::uvm_phase& phase);
  void connect_phase(uvm::uvm_phase& phase);
  void run_phase(uvm::uvm_phase& phase);

private:
  int num_mst;
  int num_slv;
  std::map<uint64_t, uint8_t> mem_expected;

  int num_reads;
  int num_writes;

  bool disabled;
};

#endif /* __CROSSBAR_SCOREBOARD_H__ */
