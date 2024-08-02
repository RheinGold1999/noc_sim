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
    ELABORATE,

    TRANSFER,
    PROCESS,
    UPDATE,

    FINALIZE,
  };

  enum class ModelType
  {
    UNKNOWN = 0,
    MODULE,
    PORT,
    REGISTER,
    STORAGE,
    MONITOR,
    TOP,
  };

public:
  ModelBase(
    const ModelBase* parent, 
    const std::string& name
  );
  virtual ~ModelBase();
  
  virtual void transfer() = 0;  // invoked in _cyc_phase_1
  virtual void process() = 0;   // invoked in _cyc_phase_2
  virtual void update() = 0;    // invoked in _cyc_phase_3

  virtual void elaborate();     // before simulation
  virtual void finalize();      // after simulation

  std::string full_name() const;
  std::string base_name() const;

  void add_child(ModelBase* child) const;

  CycStage get_stage() const;
  std::string get_stage_str() const;

  void set_logger(const Logger* logger);

  static std::list<ModelBase*>
  walk_tree_node(ModelBase* node);

protected:
  virtual void _cyc_phase_1() final;
  virtual void _cyc_phase_2() final;
  virtual void _cyc_phase_3() final;

  virtual void _elaborate() final;
  virtual void _finalize() final;

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

#endif  /* __MODEL_BASE_H__ */
