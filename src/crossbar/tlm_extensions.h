#ifndef __TLM_EXTENSIONS_H__
#define __TLM_EXTENSIONS_H__

#include <tlm>

class extension_trans_id
: public tlm::tlm_extension<extension_trans_id>
{
  friend class tlm_gp_mm;

public:
  extension_trans_id(uint64_t id);
  void copy_from(const tlm::tlm_extension_base& ext);
  tlm::tlm_extension_base* clone() const;

  uint64_t get_id() const;

private:
  uint64_t m_trans_id{0};
};


#endif /* __TLM_EXTENSIONS_H__ */
