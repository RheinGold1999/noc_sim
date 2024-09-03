#ifndef __CROSSBAR_MASTER_SEQ_LIB_H__
#define __CROSSBAR_MASTER_SEQ_LIB_H__

#include <systemc>
#include <uvm>

#include "crossbar_transfer.h"

// class crossbar_transfer;

// -----------------------------------------------------------------------------
// CROSSBAR_BASE_SEQUENCE
// -----------------------------------------------------------------------------
class crossbar_base_sequence : public uvm::uvm_sequence<crossbar_transfer>
{
  using uvm_sequence<crossbar_transfer>::uvm_report;
  using uvm_sequence<crossbar_transfer>::uvm_report_info;
  using uvm_sequence<crossbar_transfer>::uvm_report_warning;
  using uvm_sequence<crossbar_transfer>::uvm_report_error;
  using uvm_sequence<crossbar_transfer>::uvm_report_fatal;

public:
  crossbar_base_sequence(const std::string& name = "crossbar_base_seq")
  : uvm::uvm_sequence<crossbar_transfer>(name)
  {
    set_automatic_phase_objection(true);
  }

  UVM_OBJECT_UTILS(crossbar_base_sequence);
};

// -----------------------------------------------------------------------------
// SEQUENCE: READ_WORD
// -----------------------------------------------------------------------------
class read_word_seq : public crossbar_base_sequence
{
public:
  uint64_t start_addr;

  crossbar_transfer* req{nullptr};
  crossbar_transfer* rsp{nullptr};

  read_word_seq(const std::string& name = "read_word_seq")
  : crossbar_base_sequence(name)
  , start_addr(0)
  {
    req = crossbar_transfer::type_id::create();
    rsp = crossbar_transfer::type_id::create();
  }

  ~read_word_seq()
  {
    crossbar_transfer::type_id::destroy(req);
    crossbar_transfer::type_id::destroy(rsp);
  }

  UVM_OBJECT_UTILS(read_word_seq);

  virtual void body()
  {
    req->id = crossbar_transfer::alloc_id();
    req->addr = rand() & (~0b11ULL) & 0xFFFF'FFFF;
    req->cmd = READ;
    req->data.resize(4);

    UVM_INFO("START_REQ", req->convert2string(), uvm::UVM_MEDIUM);
    start_item(req);
    finish_item(req);

    get_response(rsp);
    UVM_INFO("GET_RSP", rsp->convert2string(), uvm::UVM_MEDIUM);
  }
};

// -----------------------------------------------------------------------------
// SEQUENCE: WRITE_WORD
// -----------------------------------------------------------------------------
class write_word_seq : public crossbar_base_sequence
{
public:
  uint64_t start_addr;

  crossbar_transfer* req{nullptr};
  crossbar_transfer* rsp{nullptr};

  write_word_seq(const std::string& name = "read_word_seq")
  : crossbar_base_sequence(name)
  , start_addr(0)
  {
    req = crossbar_transfer::type_id::create();
    rsp = crossbar_transfer::type_id::create();
  }
  
  ~write_word_seq()
  {
    crossbar_transfer::type_id::destroy(req);
    crossbar_transfer::type_id::destroy(rsp);
  }

  UVM_OBJECT_UTILS(read_word_seq);

  virtual void body()
  {
    req->id = crossbar_transfer::alloc_id();
    req->addr = rand() & (~0b11ULL) & 0xFFFF'FFFF;
    req->cmd = WRITE;
    req->data.resize(4);
    for (int i = 0; i < 4; ++i) {
      req->data[i] = rand() % 256;
    }

    UVM_INFO("start req", req->convert2string(), uvm::UVM_MEDIUM);
    start_item(req);
    finish_item(req);

    get_response(rsp);
    UVM_INFO("get rsp", rsp->convert2string(), uvm::UVM_MEDIUM);
  }
};

#endif /* __CROSSBAR_MASTER_SEQ_LIB_H__ */
