#ifndef __COMMUNICATION_HPP__
#define __COMMUNICATION_HPP__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <systemc>
#include <queue>

using namespace sc_core;
using namespace sc_dt;

#define BUS_WIDTH 16
#define ADDRESS_WIDTH 3
#define NUMBER_OF_ADDRESSES 2 << ADDRESS_WIDTH
#define BUS_DELAY 10 /* FIXME */

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

/* Generic dynamic list */
typedef struct queue{
  unsigned short * datum;
  sc_uint<ADDRESS_WIDTH> address;
  bool cmd;
  int id;
} queue_element;

/*
  Initiator
 */
struct Initiator: sc_module
{
  tlm_utils::simple_initiator_socket<Initiator> socket;
  std::queue<queue_element> initiator_queue;

  SC_CTOR(Initiator)
  : socket("socket")  // Construct and name socket
  {
    SC_THREAD(thread_process);
  }
  
  queue_element queue;
  sc_event write_req;

  void write(int adr, int datum, bool cmd, int old_id=-1);

  void thread_process();
};

/*
  Target
 */
struct Target: sc_module
{
  unsigned short incoming_buffer;
  unsigned short module_address;
  unsigned short id_extension;
  unsigned short destination_address;
  bool transfer_package;
  bool command;
  sc_event new_package;

  tlm_utils::simple_target_socket<Target> socket;

  SC_CTOR(Target)
  : socket("socket")
  {
    socket.register_b_transport(this, &Target::b_transport);
  }

  virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay );
};

#endif
