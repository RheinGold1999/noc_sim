#ifndef __CROSSBAR_TRANSFER_H__
#define __CROSSBAR_TRANSFER_H__

#include <tlm>
#include <uvm>

typedef enum {
  NOP,
  READ,
  WRITE
} crossbar_transfer_cmd_e;

extern const char* crossbar_transfer_cmd_str[];

class crossbar_transfer : public uvm::uvm_sequence_item
{
  using uvm_sequence_item::uvm_report;
  using uvm_sequence_item::uvm_report_info;
  using uvm_sequence_item::uvm_report_warning;
  using uvm_sequence_item::uvm_report_error;
  using uvm_sequence_item::uvm_report_fatal;

public:
  crossbar_transfer_cmd_e cmd;
  uint64_t addr;
  std::vector<uint8_t> data;
  size_t size() const;

  UVM_OBJECT_UTILS(crossbar_transfer);

  crossbar_transfer(const std::string& name);
  virtual void do_print(const uvm::uvm_printer& printer) const;
  virtual void do_pack(uvm::uvm_packer& p) const;
  virtual void do_unpack(uvm::uvm_packer& p);
  virtual void do_copy(const uvm::uvm_object& rhs);

  virtual bool do_compare(
    const uvm::uvm_object& rhs, 
    const uvm::uvm_comparer* compare = nullptr
  ) const;

  std::string convert2string() const;

  void init_by_gp(const tlm::tlm_generic_payload& trans);
  void copy_to_gp(tlm::tlm_generic_payload& trans);
};

#endif /* __CROSSBAR_TRANSFER_H__ */
