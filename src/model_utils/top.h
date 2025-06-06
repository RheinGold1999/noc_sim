#ifndef __TOP_H__
#define __TOP_H__

#include <list>

#include "model_utils/model_base.h"

class Logger;
class PortBase;
class MonitorBase;

class Top
  : public ModelBase
{
  friend class Simulator;

private:
  Top();

  Top(const Top&) = delete;
  Top& operator = (const Top&) = delete;
  Top(Top&&) = delete;

public:
  void register_unbound_port(const PortBase* port);
  void register_unbound_monitor(const MonitorBase* monitor);

private:
  void transfer() override;
  void process() override;
  void update() override;

  void elaborate() override;
  // void finalize() override;

private:
  std::list<const PortBase*> m_unbound_port_list;
  std::list<const MonitorBase*> m_unbound_monitor_list;

public:
  static Top* instance();
  static const Logger* logger;
};

#endif /* __TOP_H__ */
