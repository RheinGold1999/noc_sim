#ifndef __TLM_EXTENSIONS_H__
#define __TLM_EXTENSIONS_H__

#include <tlm>

class extension_trans_id
: public tlm::tlm_extension<extension_trans_id>
{
  friend class tlm_gp_mm;

  typedef unsigned long long trans_id_t;

public:
  extension_trans_id(trans_id_t id);
  void copy_from(const tlm::tlm_extension_base& ext);
  tlm::tlm_extension_base* clone() const;

  trans_id_t get_id() const;

private:
  trans_id_t m_trans_id{0};
};


#endif /* __TLM_EXTENSIONS_H__ */
