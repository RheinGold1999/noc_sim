
#include "crossbar_transfer.h"

const char* crossbar_transfer_cmd_str[] = {
  "NOP",
  "READ",
  "WRITE"
};

crossbar_transfer::crossbar_transfer(const std::string& name)
: uvm::uvm_sequence_item(name)
, cmd(NOP)
, addr(0)
, data(std::vector<uint8_t>())
{

}

std::size_t
crossbar_transfer::size() const
{
  return data.size();
}

void
crossbar_transfer::do_print(const uvm::uvm_printer& printer) const
{
  printer.print_string("cmd", crossbar_transfer_cmd_str[cmd]);
  printer.print_field_int("addr", addr);
  printer.print_field_int("size", size());

  for (int i = 0; i < size(); ++i) {
    char data_id[20];
    std::sprintf(data_id, "data[%d]", i);
    printer.print_field_int(data_id, data[i]);
  }
}

void
crossbar_transfer::do_pack(uvm::uvm_packer& p) const
{
  p << addr;
  p << (int)cmd;
  p << size();
  
  for (std::size_t i = 0; i < size(); ++i) {
    p << data[i];
  }
}

void
crossbar_transfer::do_unpack(uvm::uvm_packer& p)
{
  int tmp_cmd;
  p >> addr;
  p >> tmp_cmd;
  cmd = (crossbar_transfer_cmd_e)tmp_cmd;
  std::size_t tmp_size;
  p >> tmp_size;

  data.resize(tmp_size);
  for (std::size_t i = 0; i < tmp_size; ++i) {
    p >> data[i];
  } 
}

void
crossbar_transfer::do_copy(const uvm::uvm_object& rhs)
{
  const crossbar_transfer* drhs = dynamic_cast<const crossbar_transfer*>(&rhs);

  if (drhs == nullptr) {
    UVM_FATAL("DO_COPY", "Object not of type crossbar_transfer");
  }

  cmd = drhs->cmd;
  addr = drhs->addr;
  
  data.resize(drhs->size());
  for (std::size_t i = 0; i < drhs->size(); ++i) {
    data[i] = drhs->data[i];
  }
}

bool
crossbar_transfer::do_compare(
  const uvm::uvm_object& rhs,
  const uvm::uvm_comparer* comparer
) const
{
  const crossbar_transfer* drhs = dynamic_cast<const crossbar_transfer*>(&rhs);

  if (drhs == nullptr) {
    UVM_FATAL("DO_COMPARE", "Object not of type crossbar_transfer");
  }

  if (cmd != drhs->cmd || addr != drhs->addr || size() != drhs->size()) {
    return false;
  }

  for (std::size_t i = 0; i < size(); ++i) {
    if (data[i] != drhs->data[i]) {
      return false;
    }
  }

  return true;
}

std::string
crossbar_transfer::convert2string() const
{
  std::ostringstream str;
  str << "cmd: " << crossbar_transfer_cmd_str[cmd];
  str << "addr: " << addr;
  str << "size: " << size();
  for (std::size_t i = 0; i < size(); ++i) {
    str << "data[" << i << "]: " << data[i];
  }

  return str.str();
}

void
crossbar_transfer::init_by_gp(const tlm::tlm_generic_payload& gp)
{
  cmd = gp.is_read() ? READ : (
        gp.is_write() ? WRITE : NOP );

  addr = (uint64_t) gp.get_address();

  data.resize(gp.get_data_length());
  for (std::size_t i = 0; i < data.size(); ++i) {
    data[i] = gp.get_data_ptr()[i];
  }
}

void
crossbar_transfer::copy_to_gp(tlm::tlm_generic_payload& gp)
{
  switch (cmd) {
    case READ: gp.set_read(); break;
    case WRITE: gp.set_write(); break;
    default: gp.set_command(tlm::TLM_IGNORE_COMMAND); break;
  }

  gp.set_address(addr);
  gp.set_data_ptr(const_cast<uint8_t*>(data.data()));
  gp.set_data_length(size());
}

