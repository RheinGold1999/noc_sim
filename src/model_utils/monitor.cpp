#include "model_utils/monitor.h"
#include "model_utils/port.h"

void
MonitorBase::bind(PortBase* port)
{
  port->m_monitor_list.emplace_back(this);
  set_bound();
}