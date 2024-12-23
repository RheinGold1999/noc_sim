#ifndef __ADDR_DECODER_H__
#define __ADDR_DECODER_H__

#include <cstdint>
#include <vector>
#include <string>

#include <systemc>

struct AddrMapRule 
{
  uint64_t base_addr;
  uint64_t mask;
  uint32_t id;

  AddrMapRule(uint64_t base_addr_, uint64_t mask_, uint32_t id_);
  std::string to_str() const;
};

class AddrDecoder : sc_core::sc_object
{
public:
  AddrDecoder(const std::vector<AddrMapRule>& rules);
  AddrDecoder(const AddrDecoder& other);
  AddrDecoder& operator = (const AddrDecoder& other);

  AddrMapRule get_matched_rule(uint64_t ori_addr) const;
  void add_map_rule(uint64_t base_addr, uint64_t mask, uint32_t id);

public:
  std::vector<AddrMapRule> m_map_rule_vec;
};

extern AddrDecoder g_default_addr_decoder;

#endif  // __ADDR_DECODER_H__
