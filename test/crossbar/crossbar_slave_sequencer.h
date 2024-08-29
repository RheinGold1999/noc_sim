#ifndef __CROSSBAR_SLAVE_SEQUENCER_H__
#define __CROSSBAR_SLAVE_SEQUENCER_H__

#include <uvm>

#include "crossbar_transfer.h"

class crossbar_slave_sequencer : public uvm::uvm_sequencer<crossbar_transfer>
{
public:
  // Used for its sequence to generate response
  uvm::uvm_blocking_peek_port<crossbar_transfer> req_peek_port{"req_peek_port"};

public:
  crossbar_slave_sequencer(const uvm::uvm_component_name& name);

  UVM_COMPONENT_UTILS(crossbar_slave_sequencer);
};

#endif /* __CROSSBAR_SLAVE_SEQUENCER_H__ */
