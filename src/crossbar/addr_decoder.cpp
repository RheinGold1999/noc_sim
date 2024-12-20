#include "addr_decoder.h"
#include <systemc>

AddrMapRule::AddrMapRule(uint64_t base_addr_, uint64_t mask_, uint32_t id_)
: base_addr(base_addr_)
, mask(mask_)
, id(id_)
{}

AddrDecoder::AddrDecoder(const std::vector<AddrMapRule>& rules)
: m_map_rule_vec(rules)
{

}

AddrDecoder::AddrDecoder(const AddrDecoder& other)
: m_map_rule_vec(other.m_map_rule_vec)
{

}

AddrDecoder&
AddrDecoder::operator = (const AddrDecoder& other)
{
  this->m_map_rule_vec = other.m_map_rule_vec;
  return *this;
}

AddrMapRule
AddrDecoder::get_matched_rule(uint64_t addr) const
{
  for (
    auto it = m_map_rule_vec.begin()
  ; it != m_map_rule_vec.end()
  ;  ++it
  ) {
    if ((addr & (~(it->mask))) == it->base_addr) {
      return *it;
    }
  }
  sc_assert(false && "address decoding failed");
  return m_map_rule_vec.back();
}

void
AddrDecoder::add_map_rule(uint64_t base_addr_, uint64_t mask_, uint32_t id_)
{
  m_map_rule_vec.emplace_back(AddrMapRule(base_addr_, mask_, id_));
}

AddrDecoder g_default_addr_decoder = AddrDecoder(
  {
    AddrMapRule(0x0000'0000, 0xFFF'FFFF, 0),
    AddrMapRule(0x1000'0000, 0xFFF'FFFF, 1),
    AddrMapRule(0x2000'0000, 0xFFF'FFFF, 2),
    AddrMapRule(0x3000'0000, 0xFFF'FFFF, 3),
    AddrMapRule(0x4000'0000, 0xFFF'FFFF, 5),
    AddrMapRule(0x5000'0000, 0xFFF'FFFF, 6),
    AddrMapRule(0x0000'0000, UINT64_MAX, 7),
  }
);
