#include "crossbar_master_sequencer.h"

using namespace uvm;

crossbar_master_sequencer::crossbar_master_sequencer(const uvm_component_name& name)
: uvm_sequencer<crossbar_transfer>(name)
{}
