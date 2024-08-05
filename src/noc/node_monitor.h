#ifndef __NODE_MONITOR_H__
#define __NODE_MONITOR_H__

#include "model_utils/monitor.h"
#include "noc/data_type.h"

class NodeMonitor : public Monitor<Flit*>
{
public:
  NodeMonitor(const ModelBase* parent, const std::string& name);
  ~NodeMonitor();

  void read_callback(Flit* flit, int id) override;
  void write_callback(Flit* flit, int id) override;
};

#endif /* __NODE_MONITOR_H__ */
