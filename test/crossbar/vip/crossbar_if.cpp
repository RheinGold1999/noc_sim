#include "crossbar_if.h"

using namespace sc_core;
using namespace tlm;
using namespace uvm;

crossbar_if::crossbar_if(sc_module_name name)
: sc_module(name)
, m_name(name)
{
  req_get_export.bind(req_fifo);
  rsp_get_export.bind(rsp_fifo);

  target_socket.register_nb_transport_fw(this, &crossbar_if::nb_transport_fw);
  initiator_socket.register_nb_transport_bw(this, &crossbar_if::nb_transport_bw);
}

std::string
crossbar_if::get_name()
{
  return m_name;
}

tlm_sync_enum
crossbar_if::nb_transport_fw(
  tlm_generic_payload& gp,
  tlm_phase& phase,
  sc_time& time
)
{
  tlm_phase pre_phase = phase;
  tlm_sync_enum status = initiator_socket->nb_transport_fw(gp, phase, time);

  std::ostringstream msg;
  msg << "pre_phase : " << pre_phase.get_name()
      << ", phase : " << phase.get_name();
  UVM_INFO(get_name() + " DETECT_BW_TRANS", msg.str(), UVM_MEDIUM);

  if (pre_phase == BEGIN_REQ && phase == END_REQ) {
    crossbar_transfer xbar_req{"req"};
    xbar_req.init_by_gp(gp);
    bool success = req_fifo.nb_put(xbar_req);
    UVM_INFO(get_name() + " DETECT_FW_REQ", xbar_req.convert2string(), UVM_MEDIUM);
    sc_assert(success);
  } else if (pre_phase == END_RESP) {
    crossbar_transfer xbar_rsp{"rsp"};
    xbar_rsp.init_by_gp(gp);
    bool success = rsp_fifo.nb_put(xbar_rsp);
    UVM_INFO(get_name() + " DETECT_FW_RSP", xbar_rsp.convert2string(), UVM_MEDIUM);
    sc_assert(success);
  }

  return status;
}

tlm_sync_enum
crossbar_if::nb_transport_bw(
  tlm_generic_payload& gp,
  tlm_phase& phase,
  sc_time& time
)
{
  tlm_phase pre_phase = phase;
  tlm_sync_enum status = target_socket->nb_transport_bw(gp, phase, time);

  std::ostringstream msg;
  msg << "pre_phase : " << pre_phase.get_name()
      << ", phase : " << phase.get_name();
  UVM_INFO(get_name() + " DETECT_BW_TRANS", msg.str(), UVM_MEDIUM);

  if (pre_phase == END_REQ) {
    crossbar_transfer xbar_req{"req"};
    xbar_req.init_by_gp(gp);
    bool success = req_fifo.nb_put(xbar_req);
    UVM_INFO(get_name() + " DETECT_BW_REQ", xbar_req.convert2string(), UVM_MEDIUM);
    sc_assert(success);
  } else if (pre_phase == BEGIN_RESP && phase == END_RESP) {
    crossbar_transfer xbar_rsp{"rsp"};
    xbar_rsp.init_by_gp(gp);
    bool success = rsp_fifo.nb_put(xbar_rsp);
    UVM_INFO(get_name() + " DETECT_BW_RSP", xbar_rsp.convert2string(), UVM_MEDIUM);
    sc_assert(success);
  }

  return status;
}
