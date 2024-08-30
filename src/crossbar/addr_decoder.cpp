#include "addr_decoder.h"

AddrMapRule::AddrMapRule(uint64_t sa_, uint64_t ea_, int id_)
: start_addr(sa_), end_addr(ea_), id(id_)
{}

AddrDecoder::AddrDecoder(const std::vector<AddrMapRule>& rules, int dft_id)
: m_map_rule_vec(rules)
{
  add_map_rule(0, UINT64_MAX, dft_id);
}

AddrMapRule
AddrDecoder::get_matched_rule(uint64_t ori_addr) const
{
  for (
    auto it = m_map_rule_vec.begin(); 
    it != m_map_rule_vec.end(); 
    ++it
  ) {
    if (it->start_addr <= ori_addr && ori_addr < it->end_addr) {
      return *it;
    }
  }
  return m_map_rule_vec.back();
}

void
AddrDecoder::add_map_rule(uint64_t sa_, uint64_t ea_, int id_)
{
  m_map_rule_vec.emplace_back(AddrMapRule(sa_, ea_, id_));
}

AddrDecoder g_default_addr_decoder = AddrDecoder(
  {
    AddrMapRule(0x0000'0000, 0x1000'0000, 0),
    AddrMapRule(0x1000'0000, 0x2000'0000, 1),
    AddrMapRule(0x2000'0000, 0x3000'0000, 2),
    AddrMapRule(0x3000'0000, 0x4000'0000, 3),
    AddrMapRule(0x4000'0000, 0x5000'0000, 5),
    AddrMapRule(0x5000'0000, 0x6000'0000, 6),
    AddrMapRule(0x6000'0000, 0x7000'0000, 7),
    AddrMapRule(0x7000'0000, 0x8000'0000, 8),
    AddrMapRule(0x8000'0000, 0x9000'0000, 9),
    AddrMapRule(0x9000'0000, UINT64_MAX, 10),
  }
);
