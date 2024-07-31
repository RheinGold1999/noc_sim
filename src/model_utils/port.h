#ifndef __PORT_H__
#define __PORT_H__

#include <iostream>

#include "model_utils/model_base.h"
#include "model_utils/simulator.h"
#include "log/logger.h"

class PortBase
  : public ModelBase
{
  friend class Simulator;
public:
  PortBase(const ModelBase* parent, const std::string& name)
    : ModelBase(parent, name)
  {
    m_type = ModelType::PORT;
    m_base_name = parent->base_name() + "." + name;
  }

protected:
  void transfer() override {}
  void process() override {}
  void update() override {}

  void elaborate() override
  {
    if (!is_bound()) {
      ERROR("NOT bound!!!");
      abort();
    }
  }

  bool is_bound() const { return m_is_bound; }
  void set_bound() { m_is_bound = true; }

private:
  bool m_is_bound{false};
};

template<class T>
class StreamPortOut;

template<class T>
class StreamPortIn
  : public PortBase
{
  friend class StreamPortOut<T>;
public:
  StreamPortIn(const ModelBase* parent, const std::string& name)
    : PortBase(parent, name)
  {}

  void bind(StreamPortOut<T>& port_out)
  {
    if (is_bound()) {
      ERROR("port `{}` is already bound!!!", full_name());
      abort();
    }
    m_port_out = &port_out;
    set_bound();
    port_out.m_port_in = this;
    port_out.set_bound();
  }

  void bind(StreamPortOut<T>* port_out)
  {
    bind(*port_out);
  }

  bool can_read()
  {
    if (get_stage() == CycStage::TRANSFER && m_port_out->m_vld) {
      return true;
    }
    return false;
  }

  T read()
  {
    ASSERT(can_read());
    m_port_out->m_vld = false;
    return m_port_out->m_data;
  }

private:
  StreamPortOut<T>* m_port_out{nullptr};
};


template<class T>
class StreamPortOut
  : public PortBase
{
  friend class StreamPortIn<T>;
public:
  StreamPortOut(const ModelBase* parent, const std::string& name)
    : PortBase(parent, name)
  {}

  void bind(StreamPortIn<T>& port_in)
  {
    if (is_bound()) {
      ERROR("port `{}` is already bound!!!", full_name());
      abort();
    }
    m_port_in = &port_in;
    set_bound();
    port_in.m_port_out = this;
    port_in.set_bound();
  }

  void bind(StreamPortIn<T>* port_in)
  {
    bind(*port_in);
  }

  bool can_write()
  {
    if (get_stage() == CycStage::UPDATE && !m_vld) {
      return true;
    }
    return false;
  }

  void write(const T& data)
  {
    ASSERT(can_write());
    m_data = data;
    m_vld = true;
  }

private:
  StreamPortIn<T>* m_port_in{nullptr};
  T m_data;
  bool m_vld{false};
};


template<class T>
class SignalPortOut;

template<class T>
class SignalPortIn
  : public PortBase
{
  friend class SignalPortOut<T>;
public:
  SignalPortIn(const ModelBase* parent, const std::string& name)
    : PortBase(parent, name)
  {}

  void bind(SignalPortOut<T>& port_out)
  {
    if (is_bound()) {
      ERROR("port `{}` is already bound!!!", full_name());
      abort();
    }
    m_port_out = &port_out;
    set_bound();
    port_out.m_port_in = this;
    port_out.set_bound();
  }

  void bind(SignalPortOut<T>* port_out)
  {
    bind(*port_out);
  }

  T read()
  {
    ASSERT(get_stage() == CycStage::TRANSFER);
    return m_port_out->m_data;
  }

private:
  const SignalPortOut<T>* m_port_out{nullptr};
};

template<class T>
class SingalPortOut
  : public PortBase
{
  friend class SignalPortIn<T>;
public:
  SingalPortOut(const ModelBase* parent, const std::string& name)
    : PortBase(parent, name)
  {}

  void bind(SignalPortIn<T>& port_in)
  {
    if (is_bound()) {
      ERROR("port `{}` is already bound!!!", full_name());
      abort();
    }
    m_port_in = &port_in;
    set_bound();
    port_in.m_port_out = this;
    port_in.set_bound();
  }

  void bind(SignalPortIn<T>* port_in)
  {
    bind(*port_in);
  }

  void write(const T& data)
  {
    ASSERT(get_stage() == CycStage::UPDATE);
    m_data = data;
  }

private:
  SignalPortIn<T>* m_port_in{nullptr};
  T m_data;
};


#endif  /* __PORT_H__ */
