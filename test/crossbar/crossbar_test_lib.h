#ifndef __CROSSBAR_TEST_LIB_H__
#define __CROSSBAR_TEST_LIB_H__

#include <systemc>
#include <uvm>

#include "vip/crossbar_env.h"
#include "vip/crossbar_scoreboard.h"
#include "vip/crossbar_master_seq_lib.h"
#include "vip/crossbar_slave_seq_lib.h"

class test_4m_8s : public uvm::uvm_test
{
public:
  UVM_COMPONENT_UTILS(test_4m_8s);

  crossbar_env* tb{nullptr};
  uvm::uvm_table_printer* printer{nullptr};
  bool test_pass;

  test_4m_8s(const uvm::uvm_component_name& name = "test_4m_8s")
  : uvm::uvm_test(name)
  , test_pass(true)
  {}

  virtual void build_phase(uvm::uvm_phase& phase)
  {
    uvm::uvm_test::build_phase(phase);

    // Enable transaction recording for everything
    uvm::uvm_config_db<int>::set(this, "*", "recording_detail", uvm::UVM_FULL);

    tb = crossbar_env::type_id::create("tb", this);
    assert(tb);
    
    const int num_mst = 4;
    const int num_slv = 8;
    uvm::uvm_config_db<int>::set(this, "tb", "num_mst", num_mst);
    uvm::uvm_config_db<int>::set(this, "tb", "num_slv", num_slv);

    printer = new uvm::uvm_table_printer();
    printer->knobs.depth = 3;

    // Set default sequence
    for (int i = 0; i < num_mst; ++i) {
      char mst_name[20];
      std::sprintf(mst_name, "masters[%d]", i);
      uvm::uvm_config_db<uvm::uvm_object_wrapper*>::set(
        this,
        std::string("tb.") + mst_name + ".sqr.main_phase",
        "default_sequence",
        read_word_seq::type_id::get()
      );
    }

    for (int i = 0; i < num_slv; ++i) {
      char slv_name[20];
      std::sprintf(slv_name, "slaves[%d]", i);
      uvm::uvm_config_db<uvm::uvm_object_wrapper*>::set(
        this,
        std::string("tb.") + slv_name + ".sqr.run_phase",
        "default_sequence",
        slave_mem_seq::type_id::get()
      );     
    }
  }

  void end_of_elaboration_phase(uvm::uvm_phase& phase)
  {
    UVM_INFO(get_type_name(), "Printing the test topology :\n" +
      this->sprint(printer), uvm::UVM_LOW);
  }

  void run_phase(uvm::uvm_phase& phase)
  {
    sc_core::sc_time drain_time = sc_core::sc_time(100, sc_core::SC_NS);
    phase.get_objection()->set_drain_time(this, drain_time);
  }

  void extract_phase(uvm::uvm_phase& phase)
  {
    if (tb->sbd->get_err_num()) {
      test_pass = false;
    }
  }

  void report_phase(uvm::uvm_phase& phase)
  {
    if (test_pass) {
      UVM_INFO(get_type_name(), "*** UVM TEST PASSED ***", uvm::UVM_NONE);
    } else {
      UVM_ERROR(get_type_name(), "*** UVM TEST FAILED ***");
    }
  }

  void final_phase(uvm::uvm_phase& phase)
  {
    delete printer;
  }

};

#endif /* __CROSSBAR_TEST_LIB_H__ */
