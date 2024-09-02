#include "crossbar_slave_sequencer.h"

using namespace uvm;

crossbar_slave_sequencer::crossbar_slave_sequencer(const uvm_component_name& name)
: uvm_sequencer<crossbar_transfer>(name)
{}