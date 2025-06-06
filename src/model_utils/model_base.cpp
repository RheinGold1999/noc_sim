#include <iostream>

#include "model_utils/model_base.h"
#include "model_utils/simulator.h"
#include "log/logger.h"

ModelBase::ModelBase(
  const ModelBase* parent, 
  const std::string& name
)
  : m_parent(parent),
    m_children(std::list<ModelBase *>()),
    m_full_name(parent ? (parent->full_name() + "." + name) : name),
    m_base_name(name),
    m_stage(CycStage::ELABORATE),
    m_type(ModelType::UNKNOWN)
{
  if (parent) {
    parent->add_child(this);
  }

  if (parent && parent->has_logger()) {
    m_logger = Logger::inherit_from(parent, this);
  } else {
    m_logger = Logger::create_logger(this);
  }
}

ModelBase::~ModelBase()
{
  delete m_logger;
}

std::string
ModelBase::full_name() const
{
  return m_full_name;
}

std::string
ModelBase::base_name() const
{
  return m_base_name;
}

bool
ModelBase::has_logger() const
{
  return m_logger->has_spdlogger();
}

void
ModelBase::set_logger(const Logger* logger)
{
  if (m_logger) {
    delete m_logger;
  }
  m_logger = logger;
  for (auto mdl : ModelBase::walk_tree_node(this)) {
    if (mdl->m_logger) {
      delete mdl->m_logger;
    }
    mdl->m_logger = Logger::inherit_from(this, mdl);
  }
}

void
ModelBase::add_child(ModelBase* child) const
{
  m_children.emplace_back(child);
}

/**
 * @brief The returned list contains all model nodes in the tree,
 *        but does NOT include the curr_node
 */
std::list<ModelBase*>
ModelBase::walk_tree_node(ModelBase* curr_node)
{
  std::list<ModelBase*> walk_list = {curr_node};
  std::list<ModelBase*> res_list;
  ModelBase* node{nullptr};
  while (!walk_list.empty()) {
    node = walk_list.front();
    walk_list.pop_front();
    for (auto ch : node->m_children) {
      walk_list.emplace_back(ch);
      res_list.emplace_back(ch);
    }
  }
  return res_list;
}

ModelBase::CycStage
ModelBase::get_stage() const
{
  return m_stage;
}

std::string
ModelBase::get_stage_str() const
{
  switch (m_stage) {
    case CycStage::TRANSFER: {
      return "T";
    }
    case CycStage::PROCESS: {
      return "P";
    }
    case CycStage::UPDATE: {
      return "U";
    }
    case CycStage::ELABORATE: {
      return "E";
    }
    case CycStage::FINALIZE: {
      return "F";
    }
    default: {
      return "-";
    }
  }
}

void
ModelBase::_cyc_phase_1()
{
  // TRACE("{}", "transfer()");
  m_stage = CycStage::TRANSFER;
  for (auto ch : m_children) {
    ch->_cyc_phase_1();
  }
  transfer();
}

void
ModelBase::_cyc_phase_2()
{
  // TRACE("{}", "process()");
  m_stage = CycStage::PROCESS;
  for (auto ch : m_children) {
    ch->_cyc_phase_2();
  }
  process();
}

void
ModelBase::_cyc_phase_3()
{
  // TRACE("{}", "update()");
  m_stage = CycStage::UPDATE;
  for (auto ch : m_children) {
    ch->_cyc_phase_3();
  }
  update();
}

void
ModelBase::_elaborate()
{
  m_stage = CycStage::ELABORATE;
  for (auto ch : m_children) {
    ch->_elaborate();
  }
  elaborate();
}

void
ModelBase::_finalize()
{
  m_stage = CycStage::FINALIZE;
  for (auto ch : m_children) {
    ch->_finalize();
  }
  finalize();
}

void
ModelBase::elaborate()
{}

void
ModelBase::finalize()
{}

