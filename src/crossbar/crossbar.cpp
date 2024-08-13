#include <sstream>

#include "crossbar/crossbar.h"

using namespace sc_core;
using namespace sc_dt;
using namespace tlm;
using namespace tlm_utils;

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
CROSSBAR::CrossBar(const sc_module_name& name, const sc_time& period) :
  sc_module(name),
  m_period(period)
{
  sc_assert(NR_OF_INITIATORS > 0);
  sc_assert(NR_OF_TARGETS > 0);

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
    target_sockets[i].register_trasnport_dbg(this, &CrossBar::transport_dbg, i);
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
    sc_time time = sc_core::SC_ZERO_TIME;
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

    // Now the request has been received by the downstream, the 
    // m_slv_req_buf[slv_id][mst_id] should be released, and 
    // m_slv_req_stall[slv_id][mst_id] should be checked.
    m_slv_req_buf[slv_id][mst_id] = nullptr;
    // If there is a stalled req, tell the upstream that it has been received.
    transaction_type* stalled_req = m_slv_req_stall[slv_id][mst_id];
    if (stalled_req != nullptr) {
      phase_type p = tlm::END_REQ;
      sc_time t = sc_core::SC_ZERO_TIME;
      sync_enum_type s = target_sockets[slv_id]->nb_transport_bw(*stalled_req, p, t);
      sc_assert(s == tlm::TLM_ACCEPTED);
      m_slv_req_buf[slv_id][mst_id] = stalled_req;
      m_slv_req_stall[slv_id][mst_id] = nullptr;
      m_mst_req_arb_event_que[mst_id].notify();
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
    phase_type phase = tlm::BEGIN_RESP;
    sc_time time = sc_core::SC_ZERO_TIME;
    sync_enum_type status =
      target_sockets[slv_id]->nb_transport_bw(*trans, phase, time);

    if (status == tlm::TLM_ACCEPTED) {
      // The rsp has not been received yet by the upstream, needs to
      // wait nb_transport_fw call with phase END_RESP
      sc_assert(phase == tlm::BEGIN_RESP);
      wait(m_slv_end_rsp_event[slv_id]);  // notified by the upstream
    } else if (status == tlm::TLM_COMPLETED) {
      // The rsp has been received by the upstream
      sc_assert(phase == tlm::END_REQ);
      auto it = m_pending_trans_map.find(trans);
      sc_assert(it != m_pending_trans_map.end());
      m_pending_trans_map.erase(it);
      trans->release();
    } else {
      // status == tlm::TLM_UPDATED is not allowed
      sc_assert(false);
    }

    // Now th response has been received by the upstream, the 
    // m_mst_rsp_buf[mst_id][slv_id] should be released, and
    // m_mst_rsp_stall[mst_id][slv_id] should be checked.
    m_mst_rsp_buf[mst_id][slv_id] == nullptr;
    // If there is stalled rsp, tell the downstream that it has been received.
    transaction_type* stalled_rsp = m_mst_rsp_stall[mst_id][slv_id];
    if (stalled_rsp != nullptr) {
      phase_type p = tlm::END_RESP;
      sc_time t = sc_core::SC_ZERO_TIME;
      sync_enum_type s = initiator_sockets[mst_id]->nb_transport_fw(*stalled_rsp, p, t);
      sc_assert(s == tlm::TLM_COMPLETED);
      m_mst_rsp_buf[mst_id][slv_id] = stalled_rsp;
      m_mst_rsp_stall[mst_id][slv_id] = nullptr;
      m_slv_rsp_arb_event_que[slv_id].notify();
    }
  }
}

TEMPLATE
void
CROSSBAR::req_arb_thread(int mst_id)
{
  static int slv_id_rr = -1;

  while (true) {
    wait(m_mst_req_arb_event_que[mst_id]);
    
    // Arbitration via Round Robin
    do {
      slv_id_rr = (slv_id_rr + 1) % NR_OF_INITIATORS;
    } while (m_slv_req_buf[slv_id_rr][mst_id] == nullptr);

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
    wait(m_slv_rsp_arb_event_que[slv_id]);

    // Arbitration via Round Robin
    do {
      mst_id_rr = (mst_id_rr + 1) % NR_OF_TARGETS;
    } while (m_mst_rsp_buf[mst_id_rr][slv_id] == nullptr);

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
  sc_time& time
)
{
  if (phase == tlm::BEGIN_REQ) {
    trans.acquire();
    ConnectionInfo connect_info(slv_id, trans.get_address());
    decode_addr(trans, connect_info);
    int mst_id = connect_info.mst_id;
    sc_assert(mst_id < NR_OF_TARGETS);
    sc_assert(m_pending_trans_map.find(&trans) == m_pending_trans_map.end());
    m_pending_trans_map[&trans] = connect_info;
    if (m_slv_req_buf[slv_id][mst_id] == nullptr) {
      m_slv_req_buf[slv_id][mst_id] = &trans;
      phase = tlm::END_REQ;
      m_mst_req_arb_event_que[mst_id].notify();
      return tlm::TLM_UPDATED;
    } else {
      m_slv_req_stall[slv_id][mst_id] = &trans;
      return tlm::TLM_ACCEPTED;
    }
  } else if (phase == tlm::END_RESP) {
    auto it = m_pending_trans_map.find(&trans);
    sc_assert(it != m_pending_trans_map.end());
    m_pending_trans_map.erase(it);
    trans.release();
    m_slv_end_rsp_event[slv_id].notify();
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
  sc_time& time
)
{
  auto it = m_pending_trans_map.find(&trans);
  sc_assert(it != m_pending_trans_map.end());
  sc_assert(it->second.mst_id == mst_id);
  if (phase == tlm::END_REQ) {
    m_mst_end_req_event[mst_id].notify();
    return TLM_ACCEPTED;
  } else if (phase == tlm::BEGIN_RESP) {
    int slv_id = it->second.slv_id;
    trans.set_address(it->second.ori_addr);
    if (m_mst_rsp_buf[mst_id][slv_id] == nullptr) {
      m_mst_rsp_buf[mst_id][slv_id] == &trans;
      phase == tlm::END_RESP;
      m_slv_rsp_arb_event_que[slv_id].notify();
      return tlm::TLM_COMPLETED;
    } else {
      m_mst_rsp_stall[mst_id][slv_id] == &trans;
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
CROSSBAR::invalidate_dmi_ptr(int mst_id, uint64 start_range, uint64 end_range)
{
  SC_REPORT_ERROR("TLM-2", "invalidate_dmi_ptr is not supported!");
}

TEMPLATE
void
CROSSBAR::decode_addr(transaction_type& trans, ConnectionInfo& connect_info)
{
  // A simple address map, can used other types, e.g. interleave address map
  uint64 ori_addr = trans.get_address();
  if (0 <= ori_addr && ori_addr < 0x1000) {
    connect_info.mst_id = 0;
    connect_info.map_addr = ori_addr;
  } else if (0x1000'0000 <= ori_addr && ori_addr < 0x2000'0000) {
    connect_info.mst_id = 1;
    connect_info.map_addr = ori_addr - 0x1000'0000;
  } else if (0x2000'0000 <= ori_addr && ori_addr < 0x3000'0000) {
    connect_info.mst_id = 2;
    connect_info.map_addr = ori_addr - 0x2000'0000;
  } else if (0x3000'0000 <= ori_addr && ori_addr < 0x4000'0000) {
    connect_info.mst_id = 3;
    connect_info.map_addr = ori_addr - 0x3000'0000;
  } else if (0x4000'0000 <= ori_addr && ori_addr < 0x5000'0000) {
    connect_info.mst_id = 4;
    connect_info.map_addr = ori_addr - 0x4000'0000;
  } else if (0x5000'0000 <= ori_addr && ori_addr < 0x6000'0000) {
    connect_info.mst_id = 5;
    connect_info.map_addr = ori_addr - 0x5000'0000;
  } else if (0x6000'0000 <= ori_addr && ori_addr < 0x7000'0000) {
    connect_info.mst_id = 6;
    connect_info.map_addr = ori_addr - 0x6000'0000;
  } else if (0x7000'0000 <= ori_addr && ori_addr < 0x8000'0000) {
    connect_info.mst_id = 7;
    connect_info.map_addr = ori_addr - 0x7000'0000;
  } else if (0x8000'0000 <= ori_addr && ori_addr < 0x9000'0000) {
    connect_info.mst_id = 8;
    connect_info.map_addr = ori_addr - 0x8000'0000;
  } else {
    connect_info.mst_id = 9;
    connect_info.map_addr = ori_addr - 0x9000'0000;
  }
}


#undef TEMPLATE
#undef CROSSBAR
