#include "tlm_extensions.h"

extension_trans_id::extension_trans_id(trans_id_t id)
: m_trans_id(id)
{}

void
extension_trans_id::copy_from(const tlm::tlm_extension_base& ext)
{
  m_trans_id = dynamic_cast<const extension_trans_id*>(&ext)->m_trans_id;
}

tlm::tlm_extension_base*
extension_trans_id::clone() const
{
  return new extension_trans_id(*this);
}

extension_trans_id::trans_id_t
extension_trans_id::get_id() const
{
  return m_trans_id;
}

