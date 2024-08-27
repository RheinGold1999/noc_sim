#include "crossbar_if.h"

using namespace sc_core;
using namespace tlm;

crossbar_if::crossbar_if(sc_module_name name)
: sc_module(name)
{
  target_socket.register_nb_transport_fw(this, &crossbar_if::nb_transport_fw);
  initiator_socket.register_nb_transport_bw(this, &crossbar_if::nb_transport_bw);
}

tlm_sync_enum
crossbar_if::nb_transport_fw(
  tlm_generic_payload& trans,
  tlm_phase& phase,
  sc_time& time
)
{
  tlm_phase pre_phase = phase;
  tlm_sync_enum status = initiator_socket->nb_transport_fw(trans, phase, time);

  if (pre_phase == BEGIN_REQ && phase == END_REQ) {
    crossbar_transfer xbar_req{"req"};
    xbar_req.init(trans);
    bool success = req_put_port.nb_put(xbar_req);
    sc_assert(success);
  } else if (pre_phase == END_RESP) {
    crossbar_transfer xbar_rsp{"rsp"};
    xbar_rsp.init(trans);
    bool success = rsp_put_port.nb_put(xbar_rsp);
    sc_assert(success);
  }

  return status;
}

tlm_sync_enum
crossbar_if::nb_transport_bw(
  tlm_generic_payload& trans,
  tlm_phase& phase,
  sc_time& time
)
{
  tlm_phase pre_phase = phase;
  tlm_sync_enum status = target_socket->nb_transport_bw(trans, phase, time);

  if (pre_phase == END_REQ) {
    crossbar_transfer xbar_req{"req"};
    xbar_req.init(trans);
    bool success = req_put_port.nb_put(xbar_req);
    sc_assert(success);
  } else if (pre_phase == BEGIN_RESP && phase == END_RESP) {
    crossbar_transfer xbar_rsp{"rsp"};
    xbar_rsp.init(trans);
    bool success = rsp_put_port.nb_put(xbar_rsp);
    sc_assert(success);
  }

  return status;
}
