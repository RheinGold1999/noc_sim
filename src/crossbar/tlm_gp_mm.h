#ifndef __TLM_GP_MM_H__
#define __TLM_GP_MM_H__

#include <tlm>
#include <queue>

typedef unsigned long long trans_id_t;

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
  static uint64_t s_id_cnt;
  static trans_id_t get_id(const tlm::tlm_generic_payload* gp);
  static void set_id(tlm::tlm_generic_payload*gp, trans_id_t id);
};


#endif /* __TLM_GP_MM_H__ */
