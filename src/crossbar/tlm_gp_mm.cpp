#include "tlm_gp_mm.h"
#include "tlm_extensions.h"

uint64_t
tlm_gp_mm::s_id_cnt = 0;

tlm_gp_mm::tlm_gp_mm()
{}

tlm::tlm_generic_payload*
tlm_gp_mm::allocate()
{
  tlm::tlm_generic_payload* gp;
  if (!m_que.empty()) {
    gp = m_que.front();
    extension_trans_id* trans_id;
    gp->get_extension(trans_id);
    trans_id->m_trans_id = s_id_cnt++;
    m_que.pop();
  } else {
    gp = new tlm::tlm_generic_payload(this);
    extension_trans_id* trans_id = new extension_trans_id(s_id_cnt++);
    gp->set_auto_extension(trans_id);
  }
  gp->acquire();
  std::cout << "gp alloc: id : " << get_id(gp) 
            << ", ref_cnt : " << gp->get_ref_count() << std::endl;
  return gp;
}

void
tlm_gp_mm::free(tlm::tlm_generic_payload* gp)
{
  // gp->reset(); // reset will delete all extensions in gp
  std::cout << "gp free: id : " << get_id(gp) << std::endl;
  m_que.push(gp);
}

tlm_gp_mm::~tlm_gp_mm()
{
  tlm::tlm_generic_payload* gp;
  while (!m_que.empty()) {
    gp = m_que.front();
    m_que.pop();
    gp->reset();  // release all of its extensions
    delete gp;
  }
}

tlm_gp_mm&
tlm_gp_mm::instance()
{
  static tlm_gp_mm mm;
  return mm;
}

uint64_t
tlm_gp_mm::get_id(const tlm::tlm_generic_payload* gp)
{
  extension_trans_id* id = gp->get_extension<extension_trans_id>();
  assert(id);
  return id->m_trans_id;
}

void
tlm_gp_mm::set_id(tlm::tlm_generic_payload* gp, trans_id_t id)
{
  extension_trans_id* trans_id = gp->get_extension<extension_trans_id>();
  assert(trans_id);
  std::cout << "change id from " << trans_id->m_trans_id << " to " << id << std::endl;
  trans_id->m_trans_id = id;
}
