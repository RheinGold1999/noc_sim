#include "tlm_gp_mm.h"
#include "tlm_extensions.h"

// #define TLM_GP_MM_DBG

#ifdef TLM_GP_MM_DBG
  #define D(format, args...) \
    printf( \
      "%s [tlm_gp_mm.cpp:%d] [%s] " format "\n" \
    , ::sc_core::sc_time_stamp().to_string().c_str() \
    , __LINE__, __FUNCTION__, args \
    )
#else
  #define D(format, args...)
#endif

std::ostream& operator << (std::ostream& os, tlm::tlm_generic_payload& gp)
{
  os << "id: " << std::dec << tlm_gp_mm::get_id(&gp)
    << ", addr: 0x" << std::hex << gp.get_address()
    << ", data_len: " << std::dec << gp.get_data_length()
    << ", ptr: " << std::hex << (void*) (&gp)
    ;
  return os;
}

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
  D("GP_ALLOC: id: %llu, ref_cnt: %d", get_id(gp), gp->get_ref_count());
  return gp;
}

void
tlm_gp_mm::free(tlm::tlm_generic_payload* gp)
{
  // gp->reset(); // reset will delete all extensions in gp
  D("GP_FREE: id: %llu", get_id(gp));
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

trans_id_t
tlm_gp_mm::get_id(tlm::tlm_generic_payload* gp)
{
  extension_trans_id* id = gp->get_extension<extension_trans_id>();
  if(id == nullptr) {
    id = new extension_trans_id(s_id_cnt++);
    gp->set_auto_extension(id);
  }
  return id->m_trans_id;
}

void
tlm_gp_mm::set_id(tlm::tlm_generic_payload* gp, trans_id_t id)
{
  extension_trans_id* trans_id = gp->get_extension<extension_trans_id>();
  assert(trans_id);
  D("ASSIGN_NEW_ID: change id from %llu to %llu", trans_id->m_trans_id, id);
  trans_id->m_trans_id = id;
}
