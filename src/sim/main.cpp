#include <iostream>

#include "model_utils/model_base.h"
#include "model_utils/module_base.h"
#include "model_utils/port.h"
#include "model_utils/top.h"
#include "model_utils/simulator.h"
#include "log/logger.h"
#include "config/config_manager.h"


class Producer
  : public ModuleBase
{
public:
  StreamPortOut<uint32_t> out{this, "out"};

public:
  Producer(const ModelBase* parent, const std::string& name)
    : ModuleBase(parent, name),
      m_num(0)
  {}

  void transfer() override {}

  void process() override
  {
    m_num++;
  }

  void update() override
  {
    if (out.can_write()) {
      out.write(m_num);
    }
  }

private:
  uint32_t m_num;
};


class Consumer
  : public ModuleBase
{
public:
  StreamPortIn<uint32_t> in{this, "in"};

public:
  Consumer(const ModelBase* parent, const std::string& name)
    : ModuleBase(parent, name),
      m_num(0)
  {}

  void transfer() override {
    if (in.can_read()) {
      m_num = in.read();
    }
  }

  void process() override {}

  void update() override {}

private:
  uint32_t m_num;
};


int main(int argc, char** argv)
{
  ConfigManager::parse_args(argc, argv);
  ConfigManager::print_args();

  Top* top = Top::instance();
  Producer producer(top, "producer");
  Consumer consumer(top, "consumer");

  _TRACE("0");
  _DEBUG("1");
  _INFO("2");
  _WARN("3");
  _ERROR("4");
  _CRITICAL("5");

  producer.out.bind(consumer.in);
  consumer.in.bind(producer.out);

  Simulator::run(100);
  
  return 0;
}

