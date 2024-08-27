#ifndef __ADDR_DECODER_H__
#define __ADDR_DECODER_H__

#include <cstdint>
#include <vector>

struct AddrMapRule 
{
  uint64_t start_addr;
  uint64_t end_addr;
  int      id;

  AddrMapRule(uint64_t sa_, uint64_t ea_, int id_);
};

class AddrDecoder
{
public:
  AddrDecoder(const std::vector<AddrMapRule>& rules, int dft_id = 0);

  AddrMapRule get_matched_rule(uint64_t ori_addr) const;
  void add_map_rule(uint64_t start_addr, uint64_t end_addr, int id);

private:
  std::vector<AddrMapRule> m_map_rule_vec;
};

#endif /* __ADDR_DECODER_H__ */
