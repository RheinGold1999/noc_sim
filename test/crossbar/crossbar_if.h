#ifndef __CROSSBAR_IF_H__
#define __CROSSBAR_IF_H__

#include <tlm>
#include <uvm>
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "crossbar_transfer.h"

class crossbar_if : public sc_core::sc_module
{
  SC_HAS_PROCESS(crossbar_if);

public:
  typedef tlm::tlm_generic_payload transaction_type;
  typedef tlm::tlm_phase           phase_type;
  typedef tlm::tlm_sync_enum       sync_enum_type;

  typedef tlm_utils::simple_target_socket<crossbar_if>    target_socket_type;
  typedef tlm_utils::simple_initiator_socket<crossbar_if> initiator_socket_type;

public:
  target_socket_type    target_socket{"target_socket"};
  initiator_socket_type initiator_socket{"initiator_socket"};

  uvm::uvm_nonblocking_put_port<crossbar_transfer> req_put_port{"req_put_port"};
  uvm::uvm_nonblocking_put_port<crossbar_transfer> rsp_put_port{"rsp_put_port"};

  crossbar_if(sc_core::sc_module_name name);

  sync_enum_type nb_transport_fw(
    transaction_type& trans, 
    phase_type& phase, 
    sc_core::sc_time& time
  );

  sync_enum_type nb_transport_bw(
    transaction_type& trans,
    phase_type& phase,
    sc_core::sc_time& time
  );

};

#endif /* __CROSSBAR_IF_H__ */
