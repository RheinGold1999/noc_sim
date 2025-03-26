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

extension_burst_id::extension_burst_id()
{
  sc_assert(ptr == nullptr);
  ptr = new int[SIZE];
  memset(ptr, -1, sizeof(int) * SIZE);
}

extension_burst_id::extension_burst_id(const extension_burst_id& other)
{
  sc_assert(ptr == nullptr);
  sc_assert(other.ptr != nullptr);
  ptr = other.ptr;
  burst_len = other.burst_len;
}

void
extension_burst_id::copy_from(const tlm::tlm_extension_base& ext)
{
  ptr = dynamic_cast<const extension_burst_id*>(&ext)->ptr;
  burst_len = dynamic_cast<const extension_burst_id*>(&ext)->burst_len;
}

tlm::tlm_extension_base*
extension_burst_id::clone() const
{
  return new extension_burst_id(*this);
}

void
extension_burst_id::set_burst_id(int idx, int burst_id)
{
  sc_assert(ptr != nullptr);
  sc_assert(idx < SIZE);
  sc_assert(burst_id < SIZE);
  ptr[idx] = burst_id;
}

int
extension_burst_id::get_burst_id(int idx)
{
  sc_assert(idx < SIZE);
  return ptr[idx];
}

void
extension_burst_id::set_burst_len(int len)
{
  burst_len = len;
}

int
extension_burst_id::get_burst_len()
{
  return burst_len;
}

void
extension_burst_id::init()
{
  sc_assert(ptr != nullptr);
  memset(ptr, -1, sizeof(int) * SIZE);
}

void
extension_burst_id::destory()
{
  delete [] ptr;
}

