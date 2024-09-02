#ifndef __CROSSBAR_MASTER_SEQUENCER_H__
#define __CROSSBAR_MASTER_SEQUENCER_H__

#include <uvm>

#include "crossbar_transfer.h"

class crossbar_master_sequencer : public uvm::uvm_sequencer<crossbar_transfer>
{
public:
  crossbar_master_sequencer(const uvm::uvm_component_name& name);

  UVM_COMPONENT_UTILS(crossbar_master_sequencer);
};

#endif /* __CROSSBAR_MASTER_SEQUENCER_H__ */
