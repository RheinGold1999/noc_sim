#ifndef __CROSSBAR_SLAVE_SEQ_LIB_H__
#define __CROSSBAR_SLAVE_SEQ_LIB_H__

#include <uvm>
#include <map>

#include "crossbar_transfer.h"
#include "crossbar_slave_sequencer.h"

class slave_mem_seq : public uvm::uvm_sequence<crossbar_transfer>
{
public:
  slave_mem_seq(const std::string& name = "slave_mem_seq")
  : uvm::uvm_sequence<crossbar_transfer>(name)
  {}

  UVM_OBJECT_UTILS(slave_mem_seq);
  UVM_DECLARE_P_SEQUENCER(crossbar_slave_sequencer);

  virtual void pre_do(bool is_item)
  {
    rsp->id = mon_req.id;
    rsp->cmd = mon_req.cmd;
    rsp->addr = mon_req.addr;
    rsp->data.resize(mon_req.size());
    if (mon_req.cmd == READ) {
      for (int i = 0; i < mon_req.size(); ++i) {
        if (mem.find(mon_req.addr + i) == mem.end()) {
          mem[mon_req.addr + i] = rand() % 256;
        }
        rsp->data[i] = mem[mon_req.addr + i];
      }
    }
  }

  virtual void post_do(uvm::uvm_sequence_item* item)
  {
    crossbar_transfer* trans = dynamic_cast<crossbar_transfer*>(item);
    if (trans == nullptr) {
      UVM_ERROR(get_type_name(), "No valid transaction. Skipped.");
    }
    if (mon_req.cmd == WRITE && trans != nullptr) {
      for (int i = 0; i < mon_req.size(); ++i) {
        mem[mon_req.addr + i] = trans->data[i];
      }
    }
  }

  virtual void body()
  {
    UVM_INFO(get_type_name(), get_sequence_path() + " starting...", uvm::UVM_MEDIUM);

    rsp = dynamic_cast<crossbar_transfer*>(
      create_item(crossbar_transfer::get_type(), p_sequencer, "rsp")
    );

    uvm::uvm_phase* p = this->get_starting_phase();

    while (true) {
      p_sequencer->req_get_port.get(mon_req);
      if (p != nullptr) {
        p->raise_objection(this);
      }
      this->start_item(rsp);
      sc_core::wait(rand() % 50, sc_core::SC_NS);
      this->finish_item(rsp);
      if (p != nullptr) {
        p->drop_objection(this);
      }
    }
  }

private:
  std::map<uint64_t, uint8_t> mem;
  crossbar_transfer* rsp{nullptr};
  crossbar_transfer mon_req;
};


#endif /* __CROSSBAR_SLAVE_SEQ_LIB_H__ */
