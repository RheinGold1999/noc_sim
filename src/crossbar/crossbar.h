#ifndef __CROSSBAR_H__
#define __CROSSBAR_H__

#include <tlm>
#include <set>

#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

template <
  int NR_OF_INITIATORS, 
  int NR_OF_TARGETS
>
class CrossBar : public sc_core::sc_module
{
  SC_HAS_PROCESS(CrossBar);

public:
  typedef tlm::tlm_generic_payload transaction_type;
  typedef tlm::tlm_phase           phase_type;
  typedef tlm::tlm_sync_enum       sync_enum_type;

  typedef tlm_utils::simple_target_socket_tagged<CrossBar>    target_socket_type;
  typedef tlm_utils::simple_initiator_socket_tagged<CrossBar> initiator_socket_type;

public:
  target_socket_type    target_sockets[NR_OF_INITIATORS];
  initiator_socket_type initiator_sockets[NR_OF_TARGETS];

public:
  CrossBar(const sc_core::sc_module_name& name, const sc_core::sc_time& period);
  ~CrossBar();

private:
  /**
   * @brief: Try to issue request in initiator_sockets[mst_id]
   */
  void request_thread(int mst_id);

  /**
   * @brief: Try to issue response in target_sockets[slv_id]
   */
  void response_thread(int slv_id);

  /**
   * @brief: Arbitrate a request for initiator_sockets[mst_id]
   */
  void req_arb_thread(int mst_id);

  /**
   * @brief: Arbitrate a response for target_sockets[slv_id]
   */
  void rsp_arb_thread(int slv_id);

  sync_enum_type nb_transport_fw(
    int initiator_id,
    transaction_type& trans,
    phase_type& phase,
    sc_core::sc_time& t
  );

  sync_enum_type nb_transport_bw(
    int target_id,
    transaction_type& trans,
    phase_type& phase,
    sc_core::sc_time& t
  );

  unsigned int transport_dbg(int initiator_id, transaction_type& trans);
  bool get_dmi_ptr(int initiator_id, transaction_type& trans, tlm::tlm_dmi& dmi_data);
  void invalidate_dmi_ptr(int target_id, sc_dt::uint64 start_range, sc_dt::uint64 end_range);

private:
  struct ConnectionInfo
  {
    const int slv_id;
    int mst_id;
    const sc_dt::uint64 ori_addr;
    sc_dt::uint64 map_addr;
    
    ConnectionInfo(int s, sc_dt::uint64 addr) : slv_id(s), ori_addr(addr) {}
  };
  void decode_addr(transaction_type& trans, ConnectionInfo& connect_info);

private:
  sc_core::sc_time m_period;

  std::map<transaction_type*, ConnectionInfo> m_pending_trans_map;

  transaction_type* m_slv_req_buf[NR_OF_INITIATORS][NR_OF_TARGETS];
  transaction_type* m_slv_req_stall[NR_OF_INITIATORS][NR_OF_TARGETS];
  transaction_type* m_mst_rsp_buf[NR_OF_TARGETS][NR_OF_INITIATORS];
  transaction_type* m_mst_rsp_stall[NR_OF_TARGETS][NR_OF_INITIATORS];

  sc_core::sc_event_queue m_slv_rsp_arb_event_que[NR_OF_INITIATORS];
  sc_core::sc_event_queue m_mst_req_arb_event_que[NR_OF_TARGETS];
  sc_core::sc_event m_slv_begin_rsp_event[NR_OF_INITIATORS];
  sc_core::sc_event m_slv_end_rsp_event[NR_OF_INITIATORS];
  sc_core::sc_event m_mst_begin_req_event[NR_OF_TARGETS];
  sc_core::sc_event m_mst_end_req_event[NR_OF_TARGETS];
  int m_slv_arb_res[NR_OF_INITIATORS];
  int m_mst_arb_res[NR_OF_TARGETS];
};

#endif /* __CROSSBAR_H__ */
