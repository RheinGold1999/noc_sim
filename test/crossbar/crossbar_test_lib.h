#ifndef __CROSSBAR_TEST_LIB_H__
#define __CROSSBAR_TEST_LIB_H__

#include <systemc>
#include <uvm>

#include "crossbar_env.h"
#include "crossbar_scoreboard.h"

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
    
    uvm::uvm_config_db<int>::set(this, "tb", "num_mst", 4);
    uvm::uvm_config_db<int>::set(this, "tb", "num_slv", 8);

    printer = new uvm::uvm_table_printer();
    printer->knobs.depth = 3;
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
