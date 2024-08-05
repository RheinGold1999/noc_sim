#include "noc/node_monitor.h"
#include "noc/data_type.h"
#include "log/logger.h"

NodeMonitor::NodeMonitor(const ModelBase* parent, const std::string& name) :
  Monitor<Flit*>(parent, name)
{}

NodeMonitor::~NodeMonitor()
{}

void 
NodeMonitor::read_callback(Flit* flit, int id)
{

}

void 
NodeMonitor::write_callback(Flit* flit, int id)
{

}
