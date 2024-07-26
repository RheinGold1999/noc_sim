#ifndef __ROUTER_H__
#define __ROUTER_H__

#include "model_utils/module_base.h"
#include "noc/data_type.h"

/**
 * @brief `Router` serves as abstract base class for both
 *        `NodeRouter` and `BridgeRouter`
 */
class Router
  : public ModuleBase
{
public:
  Router(const ModelBase* parent, const std::string& name);
  virtual ~Router() = 0;  // make Router a abstract class

  virtual void transfer() override;
  virtual void process() override;
  virtual void update() override;

  virtual NodeAddr get_addr() const = 0;
};

#endif /* __ROUTER_H__ */
