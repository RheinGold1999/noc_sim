#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "model_utils/model_base.h"
#include "log/logger.h"

class PortBase;

class MonitorBase : 
  public ModelBase
{
public:
  MonitorBase(const ModelBase* parent, const std::string& name) :
    ModelBase(parent, name)
  {
    m_type = ModelType::MONITOR;
    m_base_name = parent->base_name() + "." + name;
  }

protected:
  void transfer() override {}
  void process() override {}
  void update() override {}

  void elaborate() override
  {
    if (!is_bound()) {
      Top::instance()->register_unbound_monitor(this);
    }
  }

  void bind(PortBase* port);

  bool is_bound() const { return m_is_bound; }
  void set_bound() { m_is_bound = true; }

private:
  bool m_is_bound{false};
};

template<class T>
class Monitor :
  public MonitorBase
{
public:
  Monitor(const ModelBase* parent, const std::string& name) :
    MonitorBase(parent, name)
  {}

  virtual void read_callback(const T& data) = 0;
  virtual void write_callback(const T& data) = 0;
};

#endif /* __MONITOR_H__ */
