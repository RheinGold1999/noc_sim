#ifndef __FIFO_H__
#define __FIFO_H__

#include <list>

#include "model_utils/model_base.h"

template <class T>
class FIFO
  : public ModelBase
{
public:
  FIFO(const ModelBase* parent, const std::string& name, int depth)
    : ModelBase(parent, name),
      m_depth(depth)
  {
    m_type = ModelType::STORAGE;
    m_base_name = parent->base_name() + "." + name;
    m_queue.clear();
  }

  void transfer() override {}
  void process() override {}
  void update() override {}

  bool can_read()
  {
    if (m_queue.size() > 0) {
      return true;
    }
    return false;
  }

  T read()
  {
    assert(can_read());
    T elem = m_queue.front();
    m_queue.pop_front()
    return elem;
  }

  bool can_write()
  {
    if (m_queue.size() < m_depth) {
      return true;
    }
    return false;
  }

  void write(const T& elem)
  {
    assert(can_write());
    m_queue.emplace_back(elem);
  }

  /**
   * @brief `remove` is only for ROB mechanism, a pure FIFO should NOT use it.
   */
  void remove(const T& elem)
  {
    m_queue.remove(T);
  }

private:
  int m_depth;
  std::list<T> m_queue;
};

#endif /* __FIFO_H__ */
