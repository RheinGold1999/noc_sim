#ifndef __TLM_GP_MM_H__
#define __TLM_GP_MM_H__

#include <tlm>
#include <queue>

class tlm_gp_mm : public tlm::tlm_mm_interface
{
private:
  // disable constructor (singleton pattern)
  tlm_gp_mm();
  ~tlm_gp_mm();

public:
  tlm::tlm_generic_payload* allocate();
  void free(tlm::tlm_generic_payload* gp);

private:
  std::queue<tlm::tlm_generic_payload*> m_que;

public:
  static tlm_gp_mm& instance();
};

tlm::tlm_generic_payload*
tlm_gp_mm::allocate()
{
  tlm::tlm_generic_payload* gp;
  if (!m_que.empty()) {
    gp = m_que.front();
    m_que.pop();
  } else {
    gp = new tlm::tlm_generic_payload(this);
  }
  return gp;
}

void
tlm_gp_mm::free(tlm::tlm_generic_payload* gp)
{
  gp->reset();
  m_que.push(gp);
}

tlm_gp_mm::~tlm_gp_mm()
{
  tlm::tlm_generic_payload* gp;
  while (!m_que.empty()) {
    gp = m_que.front();
    m_que.pop();
    delete gp;
  }
}

tlm_gp_mm&
tlm_gp_mm::instance()
{
  static tlm_gp_mm mm;
  return mm;
}

#endif /* __TLM_GP_MM_H__ */
