#ifndef __CROSSBAR_H__
#define __CROSSBAR_H__

#include <tlm>
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "addr_decoder.h"
#include "tlm_gp_mm.h"

// #define CROSSBAR_DBG

#ifndef CROSSBAR_DBG
  #define D(format, args...)
#else
  #define D(format, args...) \
    printf( \
      "[%s] [crossbar.h:%d] [%s] " format "\n" \
    , sc_core::sc_time_stamp().to_string().c_str() \
    , __LINE__, __FUNCTION__, args \
    )
#endif

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
  CrossBar(
    const sc_core::sc_module_name& name, 
    const sc_core::sc_time& period,
    const AddrDecoder* addr_dec
  );
  ~CrossBar();

private:
  /**
   * @brief Try to issue request in initiator_sockets[mst_id]
   */
  void request_thread(int mst_id);

  /**
   * @brief Try to issue response in target_sockets[slv_id]
   */
  void response_thread(int slv_id);

  /**
   * @brief Arbitrate a request for initiator_sockets[mst_id]
   */
  void req_arb_thread(int mst_id);

  /**
   * @brief Arbitrate a response for target_sockets[slv_id]
   */
  void rsp_arb_thread(int slv_id);

  // ---------------------------------------------------------------------------
  // TLM-2.0 Interfaces
  // ---------------------------------------------------------------------------
  sync_enum_type nb_transport_fw(
    int initiator_id,
    transaction_type& trans,
    phase_type& phase,
    sc_core::sc_time& time
  );

  sync_enum_type nb_transport_bw(
    int target_id,
    transaction_type& trans,
    phase_type& phase,
    sc_core::sc_time& time
  );

  unsigned int transport_dbg(int initiator_id, transaction_type& trans);
  bool get_dmi_ptr(int initiator_id, transaction_type& trans, tlm::tlm_dmi& dmi_data);
  void invalidate_dmi_ptr(int target_id, sc_dt::uint64 start_range, sc_dt::uint64 end_range);

private:
  class ConnectionInfo
  {
  public:
    ConnectionInfo(int s, sc_dt::uint64 addr) : slv_id(s), ori_addr(addr) {}
    ConnectionInfo() : slv_id(-1), ori_addr(0) {}

    int get_slv_id() { return slv_id; }  
    sc_dt::uint64 get_ori_addr() { return ori_addr; }

  private:
    int slv_id;
    sc_dt::uint64 ori_addr;
  public:
    int mst_id;
    sc_dt::uint64 map_addr;
  };

  void decode_addr(transaction_type& trans, ConnectionInfo& connect_info);

private:
  sc_core::sc_time m_period;
  std::map<trans_id_t, ConnectionInfo> m_pending_trans_map;
  const AddrDecoder* m_addr_dec;

  /** 
   * @brief Store requests from the upstream in slave side, each slave 
   *         has a unique slot for a single destination (master).
  */
  transaction_type* m_slv_req_buf[NR_OF_INITIATORS][NR_OF_TARGETS];

  /**
   * @brief Store requests that are stalled currently (without ready 
   *         asserted), just for the purpose of notifying the corresponding 
   *         upstream that the request can be received now (i.e. 
   *         asserting ready).
   */
  transaction_type* m_slv_req_stall[NR_OF_INITIATORS][NR_OF_TARGETS];

  /**
   * @brief Store response from the downstream in master side, each master
   *         has a unique slot for a single destination (slave).
   */
  transaction_type* m_mst_rsp_buf[NR_OF_TARGETS][NR_OF_INITIATORS];

  /**
   * @brief Store responses that are stalled currently (without ready
   *         asserted), just for the purpose of notifying the corresponding
   *         downstream that the response can be received now (i.e. 
   *         asserting ready).
   */
  transaction_type* m_mst_rsp_stall[NR_OF_TARGETS][NR_OF_INITIATORS];

  /**
   * @brief Used to notify the arbiter for a given slave that there
   *         is response coming from the downstream, and the arbiter 
   *         should pick one to relay to the upstream.
   */
  sc_core::sc_event_queue m_slv_rsp_arb_event_que[NR_OF_INITIATORS];

  /**
   * @brief Used to notify the arbiter for a given master taht there
   *         is request coming from the upstream, and the arbiter
   *         should pick one to relay to the downstream.
   */
  sc_core::sc_event_queue m_mst_req_arb_event_que[NR_OF_TARGETS];

  /**
   * @brief Used to notify the slave that it can send response
   *         to the corresponding upstream. The event is triggered
   *         by the related response arbiter.
   */
  sc_core::sc_event m_slv_begin_rsp_event[NR_OF_INITIATORS];

  /**
   * @brief Used to notify the slave that the stalled response
   *         has been received by the corresponding upstream, so 
   *         that the slave can proceed sending response.
   */
  sc_core::sc_event m_slv_end_rsp_event[NR_OF_INITIATORS];

  /**
   * @brief Used to notify the master that is can send request
   *         to the corresponding downstream. The event is triggered
   *         by the related request arbiter.
   */
  sc_core::sc_event m_mst_begin_req_event[NR_OF_TARGETS];

  /**
   * @brief Used to notify the master that the stalled request
   *         has been received by the corresponding downstream,
   *         so that the master can proceed sending request.
   */
  sc_core::sc_event m_mst_end_req_event[NR_OF_TARGETS];

  /**
   * @brief Store the mst_id selected by the response arbiter for 
   *         a given slave.
   */
  int m_slv_rsp_arb_res[NR_OF_INITIATORS];

  /**
   * @brief Store the slv_id selected by the request arbiter for 
   *         a given master.
   */
  int m_mst_req_arb_res[NR_OF_TARGETS];
};

// -----------------------------------------------------------------------------
// Implementation
// -----------------------------------------------------------------------------

#define TEMPLATE \
  template < \
    int NR_OF_INITIATORS, \
    int NR_OF_TARGETS \
  >

#define CROSSBAR \
  CrossBar< \
    NR_OF_INITIATORS, \
    NR_OF_TARGETS \
  >

TEMPLATE
CROSSBAR::CrossBar(
  const sc_core::sc_module_name& name, 
  const sc_core::sc_time& period,
  const AddrDecoder* addr_dec
)
: sc_module(name)
, m_period(period)
, m_addr_dec(addr_dec)
{
  sc_assert(NR_OF_INITIATORS > 0);
  sc_assert(NR_OF_TARGETS > 0);
  sc_assert(m_addr_dec);

  // ---------------------------------------------------------------------------
  // Initialize all the buffers (implemented as transaction pointer arrays)
  // ---------------------------------------------------------------------------
  for (int i = 0; i < NR_OF_INITIATORS; ++i) {
    for (int j = 0; j < NR_OF_TARGETS; ++j) {
      m_slv_req_buf[i][j] = nullptr;
      m_slv_req_stall[i][j] = nullptr;
    }
  }

  for (int i = 0; i < NR_OF_TARGETS; ++i) {
    for (int j = 0; j < NR_OF_INITIATORS; ++j) {
      m_mst_rsp_buf[i][j] = nullptr;
      m_mst_rsp_stall[i][j] = nullptr;
    }
  }

  // ---------------------------------------------------------------------------
  // Register sockets
  // ---------------------------------------------------------------------------
  for (int i = 0; i < NR_OF_INITIATORS; ++i) {
    target_sockets[i].register_nb_transport_fw(this, &CrossBar::nb_transport_fw, i);
    target_sockets[i].register_transport_dbg(this, &CrossBar::transport_dbg, i);
    target_sockets[i].register_get_direct_mem_ptr(this, &CrossBar::get_dmi_ptr, i);
  }

  for (int i = 0; i < NR_OF_TARGETS; ++i) {
    initiator_sockets[i].register_nb_transport_bw(this, &CrossBar::nb_transport_bw, i);
    initiator_sockets[i].register_invalidate_direct_mem_ptr(this, &CrossBar::invalidate_dmi_ptr, i);
  }

  // ---------------------------------------------------------------------------
  // Register SystemC threads
  // ---------------------------------------------------------------------------
  std::ostringstream os;
  for (int slv_id = 0; slv_id < NR_OF_INITIATORS; ++slv_id) {
    os.str("");
    os << "response_thread_" << slv_id;
    sc_spawn(sc_bind(&CrossBar::response_thread, this, slv_id), os.str().c_str());
    
    os.str("");
    os << "rsp_arb_thread_" << slv_id;
    sc_spawn(sc_bind(&CrossBar::rsp_arb_thread, this, slv_id), os.str().c_str());
  }

  for (int mst_id = 0; mst_id < NR_OF_TARGETS; ++mst_id) {
    os.str("");
    os << "request_thread_" << mst_id;
    sc_spawn(sc_bind(&CrossBar::request_thread, this, mst_id), os.str().c_str());
    
    os.str("");
    os << "req_arb_thread_" << mst_id;
    sc_spawn(sc_bind(&CrossBar::req_arb_thread, this, mst_id), os.str().c_str());
  }
}

TEMPLATE
CROSSBAR::~CrossBar()
{}

/**
 * @brief: Try to issue request in initiator_sockets[mst_id]
 */
TEMPLATE
void
CROSSBAR::request_thread(int mst_id)
{
  while (true) {
    wait(m_mst_begin_req_event[mst_id]);  // triggered in req_arb_thread(mst_id)

    int slv_id = m_mst_req_arb_res[mst_id];
    transaction_type* trans = m_slv_req_buf[slv_id][mst_id];
    sc_assert(trans != nullptr);
    phase_type phase = tlm::BEGIN_REQ;
    sc_core::sc_time time = sc_core::SC_ZERO_TIME;
    sync_enum_type status = 
      initiator_sockets[mst_id]->nb_transport_fw(*trans, phase, time);

    if (status == tlm::TLM_ACCEPTED) {
      // The req has not been recevied yet by the downstream, needs to
      // wait nb_transport_bw call with phase END_REQ
      sc_assert(phase == tlm::BEGIN_REQ);
      wait(m_mst_end_req_event[mst_id]);  // notified by the downstream
    } else if (status == tlm::TLM_UPDATED) {
      // The req has been received by the downstream
      sc_assert(phase == tlm::END_REQ);
    } else {
      // status == tlm::TLM_COMPLETED is not allowed
      sc_assert(false);
    }

    // Now the request has been received by the downstream, so: 
    // 1. Release the TLM request.
    trans->release();
    // 2. Release the m_slv_req_buf[slv_id][mst_id].
    m_slv_req_buf[slv_id][mst_id] = nullptr;
    // 3. If there is a stalled req, tell the upstream that it has been received.
    transaction_type* stalled_req = m_slv_req_stall[slv_id][mst_id];
    if (stalled_req != nullptr) {
      phase_type p = tlm::END_REQ;
      sc_core::sc_time t = sc_core::SC_ZERO_TIME;
      sync_enum_type s = target_sockets[slv_id]->nb_transport_bw(*stalled_req, p, t);
      sc_assert(s == tlm::TLM_ACCEPTED);
      m_slv_req_buf[slv_id][mst_id] = stalled_req;
      m_slv_req_stall[slv_id][mst_id] = nullptr;
      m_mst_req_arb_event_que[mst_id].notify(sc_core::SC_ZERO_TIME);
    }
  }
}

/**
 * @brief: Try to issue response in target_sockets[slv_id]
 */
TEMPLATE
void
CROSSBAR::response_thread(int slv_id)
{
  while (true) {
    wait(m_slv_begin_rsp_event[slv_id]);  // triggered in rsp_arb_thread(slv_id)

    int mst_id = m_slv_rsp_arb_res[slv_id];
    transaction_type* trans = m_mst_rsp_buf[mst_id][slv_id];
    sc_assert(trans != nullptr);
    trans_id_t trans_id = tlm_gp_mm::get_id(trans);
    phase_type phase = tlm::BEGIN_RESP;
    D("BEGIN_RESP: gp_id: %d, slv_id: %d, mst_id: %d", trans_id, slv_id, mst_id);
    sc_core::sc_time time = sc_core::SC_ZERO_TIME;
    sync_enum_type status =
      target_sockets[slv_id]->nb_transport_bw(*trans, phase, time);

    if (status == tlm::TLM_ACCEPTED) {
      // The rsp has not been received yet by the upstream, needs to
      // wait nb_transport_fw call with phase END_RESP
      sc_assert(phase == tlm::BEGIN_RESP);
      D("RESP_PENDING: gp_id: %d", trans_id);
      wait(m_slv_end_rsp_event[slv_id]);  // notified by the upstream
    } else if (status == tlm::TLM_COMPLETED) {
      // The rsp has been received by the upstream
      sc_assert(phase == tlm::END_RESP);
      auto it = m_pending_trans_map.find(tlm_gp_mm::get_id(trans));
      sc_assert(it != m_pending_trans_map.end());
      m_pending_trans_map.erase(it);
      D("END_RESP: gp_id: %d", trans_id);
    } else {
      // status == tlm::TLM_UPDATED is not allowed
      sc_assert(false);
    }

    // Now th response has been received by the upstream, so:
    // 1. Release the TLM Response
    trans->release();
    // 2. Release the m_mst_rsp_buf[mst_id][slv_id].
    m_mst_rsp_buf[mst_id][slv_id] = nullptr;
    D("m_mst_rsp_buf[%d][%d] = %#llx", 
      mst_id, slv_id, m_mst_rsp_buf[mst_id][slv_id]);
    // 3. If there is stalled rsp, tell the downstream that it has been received.
    transaction_type* stalled_rsp = m_mst_rsp_stall[mst_id][slv_id];
    if (stalled_rsp != nullptr) {
      phase_type p = tlm::END_RESP;
      sc_core::sc_time t = sc_core::SC_ZERO_TIME;
      sync_enum_type s = initiator_sockets[mst_id]->nb_transport_fw(*stalled_rsp, p, t);
      sc_assert(s == tlm::TLM_COMPLETED);
      D("RSP_STALL_TO_BUF: gp_id: %d", tlm_gp_mm::get_id(stalled_rsp));
      m_mst_rsp_buf[mst_id][slv_id] = stalled_rsp;
      m_mst_rsp_stall[mst_id][slv_id] = nullptr;
      m_slv_rsp_arb_event_que[slv_id].notify(sc_core::SC_ZERO_TIME);
    }
  }
}

TEMPLATE
void
CROSSBAR::req_arb_thread(int mst_id)
{
  static int slv_id_rr = -1;

  while (true) {
    wait(m_mst_req_arb_event_que[mst_id].default_event());

    D("req to arb: mst_id = %d", mst_id);
    // Arbitration via Round Robin
    do {
      slv_id_rr = (slv_id_rr + 1) % NR_OF_INITIATORS;
    } while (m_slv_req_buf[slv_id_rr][mst_id] == nullptr);

    D("req arb res: mst_id = %d, slv_id_rr = %d", mst_id, slv_id_rr);

    m_mst_req_arb_res[mst_id] = slv_id_rr;
    m_mst_begin_req_event[mst_id].notify();

    wait(m_period);
  }
}

TEMPLATE
void
CROSSBAR::rsp_arb_thread(int slv_id)
{
  static int mst_id_rr = -1;
  
  while (true) {
    wait(m_slv_rsp_arb_event_que[slv_id].default_event());

    D("rsp to arb: slv_id = %d", slv_id);
    // Arbitration via Round Robin
    do {
      mst_id_rr = (mst_id_rr + 1) % NR_OF_TARGETS;
    } while (m_mst_rsp_buf[mst_id_rr][slv_id] == nullptr);

    D("rsp arb res: slv_id = %d, mst_id_rr = %d", slv_id, mst_id_rr);

    m_slv_rsp_arb_res[slv_id] = mst_id_rr;
    m_slv_begin_rsp_event[slv_id].notify();

    wait(m_period);
  }
}

TEMPLATE
typename CROSSBAR::sync_enum_type
CROSSBAR::nb_transport_fw(
  int slv_id,
  transaction_type& trans,
  phase_type& phase,
  sc_core::sc_time& time
)
{
  trans_id_t trans_id = tlm_gp_mm::get_id(&trans);
  if (phase == tlm::BEGIN_REQ) {
    ConnectionInfo connect_info{slv_id, trans.get_address()};
    decode_addr(trans, connect_info);
    int mst_id = connect_info.mst_id;
    sc_assert(mst_id < NR_OF_TARGETS);
    sc_assert(m_pending_trans_map.find(trans_id) == m_pending_trans_map.end());
    m_pending_trans_map[trans_id] = connect_info;
    D("BEGIN_REQ: gp_id: %d, from %d to %d", trans_id, slv_id, mst_id);
    if (m_slv_req_buf[slv_id][mst_id] == nullptr) {
      m_slv_req_buf[slv_id][mst_id] = &trans;
      trans.acquire();
      phase = tlm::END_REQ;
      m_mst_req_arb_event_que[mst_id].notify(sc_core::SC_ZERO_TIME);
      D("END_REQ: gp_id: %d", trans_id);
      return tlm::TLM_UPDATED;
    } else {
      m_slv_req_stall[slv_id][mst_id] = &trans;
      trans.acquire();
      D("REQ_PENDING: gp_id: %d", trans_id);
      return tlm::TLM_ACCEPTED;
    }
  } else if (phase == tlm::END_RESP) {
    auto it = m_pending_trans_map.find(tlm_gp_mm::get_id(&trans));
    sc_assert(it != m_pending_trans_map.end());
    m_pending_trans_map.erase(it);
    m_slv_end_rsp_event[slv_id].notify(sc_core::SC_ZERO_TIME);
    D("END_RESP: gp_id: %d", trans_id);
    return tlm::TLM_COMPLETED;
  } else {
    SC_REPORT_ERROR("TLM-2", 
      "phase in nb_transport_fw call should be `BEGEN_REQ` or `END_RESP`");
    sc_assert(false);
  }
  return tlm::TLM_ACCEPTED;
}

TEMPLATE
typename CROSSBAR::sync_enum_type
CROSSBAR::nb_transport_bw(
  int mst_id,
  transaction_type& trans,
  phase_type& phase,
  sc_core::sc_time& time
)
{
  trans_id_t trans_id = tlm_gp_mm::get_id(&trans);
  auto it = m_pending_trans_map.find(trans_id);
  D("gp_id: %d, addr: %#llx", trans_id, trans.get_address());
  sc_assert(it != m_pending_trans_map.end());
  sc_assert(it->second.mst_id == mst_id);
  if (phase == tlm::END_REQ) {
    m_mst_end_req_event[mst_id].notify();
    D("END_REQ: gp_id: %d, end req", trans_id);
    return tlm::TLM_ACCEPTED;
  } else if (phase == tlm::BEGIN_RESP) {
    int slv_id = it->second.get_slv_id();
    D("BEGIN_RESP: gp_id: %d, mst_id: %d, slv_id: %d", trans_id, mst_id, slv_id);
    trans.set_address(it->second.get_ori_addr());
    if (m_mst_rsp_buf[mst_id][slv_id] == nullptr) {
      m_mst_rsp_buf[mst_id][slv_id] = &trans;
      trans.acquire();
      phase = tlm::END_RESP;
      m_slv_rsp_arb_event_que[slv_id].notify(sc_core::SC_ZERO_TIME);
      D("END_RESP: gp_id: %d, m_mst_rsp_buf[%d][%d]: %#llx", 
        trans_id, mst_id, slv_id, m_mst_rsp_buf[mst_id][slv_id]);
      return tlm::TLM_COMPLETED;
    } else {
      m_mst_rsp_stall[mst_id][slv_id] == &trans;
      trans.acquire();
      D("m_mst_rsp_buf[%d][%d] = %#llx", 
        mst_id, slv_id, m_mst_rsp_buf[mst_id][slv_id]);
      D("RESP_PENDING: gp_id: %d, pending gp_id: %d", 
        trans_id, tlm_gp_mm::get_id(m_mst_rsp_buf[mst_id][slv_id]));
      return tlm::TLM_ACCEPTED;
    }
  } else {
    SC_REPORT_ERROR("TLM-2", 
      "phase in nb_transport_bw call should be `END_REQ` or `BEGIN_RESP`");
    sc_assert(false);
  }
  return tlm::TLM_ACCEPTED;
}

TEMPLATE
unsigned int
CROSSBAR::transport_dbg(int slv_id, transaction_type& trans)
{
  SC_REPORT_ERROR("TLM-2", "transport_dbg is not supported!");
  return 0;
}

TEMPLATE
bool
CROSSBAR::get_dmi_ptr(int slv_id, transaction_type& trans, tlm::tlm_dmi& dmi_data)
{
  SC_REPORT_ERROR("TLM-2", "get_dmi_ptr is not supported!");
  return false;
}

TEMPLATE
void
CROSSBAR::invalidate_dmi_ptr(int mst_id, sc_dt::uint64 start_range, sc_dt::uint64 end_range)
{
  SC_REPORT_ERROR("TLM-2", "invalidate_dmi_ptr is not supported!");
}

TEMPLATE
void
CROSSBAR::decode_addr(transaction_type& trans, ConnectionInfo& connect_info)
{
  uint64_t ori_addr = (uint64_t)trans.get_address();
  AddrMapRule matched_rule = m_addr_dec->get_matched_rule(ori_addr);
  std::cout << "ori_addr = 0x" << std::hex << ori_addr << ", mst_id = " << matched_rule.id << std::endl;
  connect_info.mst_id = matched_rule.id;
  connect_info.map_addr = ori_addr - matched_rule.start_addr;
}


#undef TEMPLATE
#undef CROSSBAR

#undef D

#endif /* __CROSSBAR_H__ */
