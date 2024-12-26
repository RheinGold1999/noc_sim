
#include "crossbar_transfer.h"
#include "crossbar/tlm_gp_mm.h"

const char* crossbar_transfer_cmd_str[] = {
  "NOP",
  "READ",
  "WRITE"
};

crossbar_transfer::crossbar_transfer(const std::string& name)
: uvm::uvm_sequence_item(name)
, id(0)
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
  printer.print_field_int("id", id);
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
  p << id;
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
  p >> id;
  p >> addr;
  int tmp_cmd;
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

  id = drhs->id;
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

  if (id != drhs->id || cmd != drhs->cmd || addr != drhs->addr || size() != drhs->size()) {
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
  str << "id: " << id;
  str << " cmd: " << crossbar_transfer_cmd_str[cmd];
  str << " addr: 0x" << std::hex << addr << std::dec;
  str << " size: " << size();
  for (std::size_t i = 0; i < size(); ++i) {
    str << " data[" << i << "]: 0x" << std::hex << (uint16_t) data[i];
  }

  return str.str();
}

crossbar_transfer&
crossbar_transfer::operator=(const crossbar_transfer& rhs)
{
  if (this == &rhs) {
    return *this;
  }
  uvm::uvm_sequence_item::operator=(rhs);
  this->do_copy(rhs);
  return *this;
}

void
crossbar_transfer::init_by_gp(tlm::tlm_generic_payload& gp)
{
  id = tlm_gp_mm::get_id(&gp);
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
  tlm_gp_mm::set_id(&gp, id);
  switch (cmd) {
    case READ: gp.set_read(); break;
    case WRITE: gp.set_write(); break;
    default: gp.set_command(tlm::TLM_IGNORE_COMMAND); break;
  }

  gp.set_address(addr);
  gp.set_data_ptr(const_cast<uint8_t*>(data.data()));
  gp.set_data_length(size());
}

uint64_t
crossbar_transfer::alloc_id()
{
  static uint64_t s_glb_id = 0;
  return ++s_glb_id;
}

