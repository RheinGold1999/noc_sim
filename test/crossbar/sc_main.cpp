#include <systemc>
#include <tlm>
#include <uvm>

#include "crossbar/crossbar.h"
#include "vip/crossbar_if.h"
#include "crossbar_test_lib.h"

int sc_main(int, char**)
{
  constexpr int NUM_MST = 4;
  constexpr int NUM_SLV = 8;

  AddrDecoder* addr_dec = new AddrDecoder(
    {
      AddrMapRule(0x0000'0000, 0xFFF'FFFF, 0),
      AddrMapRule(0x1000'0000, 0xFFF'FFFF, 1),
      AddrMapRule(0x2000'0000, 0xFFF'FFFF, 2),
      AddrMapRule(0x3000'0000, 0xFFF'FFFF, 3),
      AddrMapRule(0x4000'0000, 0xFFF'FFFF, 5),
      AddrMapRule(0x5000'0000, 0xFFF'FFFF, 6),
      AddrMapRule(0x0000'0000, UINT64_MAX, 7),
    }
  );

  sc_core::sc_time period{1, sc_core::SC_NS};
  CrossBar<NUM_MST, NUM_SLV> dut("dut", period, addr_dec);
  // CrossBar<NUM_MST, NUM_SLV> dut("dut", sc_core::sc_time(1, sc_core::SC_NS));

  uvm::uvm_config_db<AddrDecoder*>::set(
    uvm::uvm_root::get(), "*.tb.sbd", "addr_dec", addr_dec
  );

  std::array<crossbar_if*, NUM_MST> mst_vifs;
  std::array<crossbar_if*, NUM_SLV> slv_vifs;

  for (int i = 0; i < NUM_MST; ++i) {
    char mst_vif_name[20];
    std::sprintf(mst_vif_name, "master_vifs[%d]", i);
    mst_vifs[i] = new crossbar_if(mst_vif_name);

    char mst_name[20];
    std::sprintf(mst_name, "masters[%d]", i);
    uvm::uvm_config_db<crossbar_if*>::set(
      uvm::uvm_root::get(), std::string("*.") + mst_name, "vif", mst_vifs[i]
    );
    mst_vifs[i]->initiator_socket.bind(dut.target_sockets[i]);
  }

  for (int i = 0; i < NUM_SLV; ++i) {
    char slv_vif_name[20];
    std::sprintf(slv_vif_name, "slave_vifs[%d]", i);
    slv_vifs[i] = new crossbar_if(slv_vif_name);

    char slv_name[20];
    std::sprintf(slv_name, "slaves[%d]", i);
    uvm::uvm_config_db<crossbar_if*>::set(
      uvm::uvm_root::get(), std::string("*.") + slv_name, "vif", slv_vifs[i]
    );
    dut.initiator_sockets[i].bind(slv_vifs[i]->target_socket);
  }

  uvm::run_test("test_4m_8s");

  for (int i = 0; i < NUM_MST; ++i) {
    delete mst_vifs[i];
  }
  for (int i = 0; i < NUM_SLV; ++i) {
    delete slv_vifs[i];
  }

  delete addr_dec;

  return 0;
}
