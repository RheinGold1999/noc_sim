#include "addr_decoder.h"
#include <systemc>
#include <sstream>


#ifdef CROSSBAR_DBG
  #define D(format, args...) \
    printf( \
      "%s [addr_decoder.cpp:%d][%s][%s] " format "\n" \
    , sc_core::sc_time_stamp().to_string().c_str() \
    , __LINE__, __FUNCTION__, this->name(), args \
    )
#else
  #define D(format, args...)
#endif

AddrMapRule::AddrMapRule(uint64_t base_addr_, uint64_t mask_, uint32_t id_)
: base_addr(base_addr_)
, mask(mask_)
, id(id_)
{}

std::string
AddrMapRule::to_str() const
{
  std::ostringstream os;
  os << "base_addr:0x" << std::hex << base_addr 
      << ", mask:0x" << mask 
      << ", id:" << std::dec << id
      ;
  return os.str();
}


AddrDecoder::AddrDecoder(const std::vector<AddrMapRule>& rules)
: sc_core::sc_object("addr_dec")
, m_map_rule_vec(rules)
{

}

AddrDecoder::AddrDecoder(const AddrDecoder& other)
: sc_core::sc_object("addr_dec")
, m_map_rule_vec(other.m_map_rule_vec)
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
  D("addr & (~mask) = %llx", (addr & (~(m_map_rule_vec.front().mask))));
  for (
    auto it = m_map_rule_vec.begin()
  ; it != m_map_rule_vec.end()
  ;  ++it
  ) {
    D("rule: %s", it->to_str().c_str());
    if ((addr & (~(it->mask))) == it->base_addr) {
      return *it;
    }
  }
  D("addr=0x%llx", addr);
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
