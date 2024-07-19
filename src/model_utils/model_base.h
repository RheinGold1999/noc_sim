#ifndef __MODEL_BASE_H__
#define __MODEL_BASE_H__

#include <string>
#include <list>

// forward declaration
class Logger;

class ModelBase
{
  friend class Simulator;

public:
  enum class CycStage
  { 
    UNKNOWN = 0,
    TRANSFER,
    PROCESS,
    UPDATE,
  };

  enum class ModelType
  {
    UNKNOWN = 0,
    MODULE,
    PORT,
    REGISTER,
    STORAGE,
    TOP,
  };

public:
  ModelBase(
    const ModelBase* parent, 
    const std::string& name
  );
  virtual ~ModelBase();
  
  virtual void transfer() = 0;
  virtual void process() = 0;
  virtual void update() = 0;

  std::string full_name() const;
  std::string base_name() const;

  void add_child(ModelBase* child) const;

  CycStage get_stage() const;

  void set_logger(const Logger* logger);

  static std::list<ModelBase*>
  walk_tree_node(ModelBase* node);

protected:
  virtual void _cyc_phase_1() final;
  virtual void _cyc_phase_2() final;
  virtual void _cyc_phase_3() final;

private:
  bool has_logger() const;

public:
  const Logger* m_logger;

protected:
  const ModelBase* m_parent;
  mutable std::list<ModelBase*> m_children;
  std::string m_full_name;
  std::string m_base_name;
  CycStage m_stage;
  ModelType m_type;
};

std::ostream& operator << (std::ostream& os, const ModelBase::CycStage& stage);

#endif  /* __MODEL_BASE_H__ */
