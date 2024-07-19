#ifndef __TOP_H__
#define __TOP_H__

#include "model_utils/model_base.h"

class Logger;

class Top
  : public ModelBase
{
  friend class Simulator;

private:
  Top();

  Top(const Top&) = delete;
  Top& operator = (const Top&) = delete;
  Top(Top&&) = delete;

private:
  void transfer() override;
  void process() override;
  void update() override;

public:
  static Top* instance();

  static const Logger* logger;
};

#endif /* __TOP_H__ */
