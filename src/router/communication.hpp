
// Filename: tlm2_getting_started_4.cpp

//----------------------------------------------------------------------
//  Copyright (c) 2007-2008 by Doulos Ltd.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//----------------------------------------------------------------------

// Version 2  19-June-2008 - updated for TLM-2.0


// Getting Started with TLM-2.0, Example 4

// Shows the non-blocking transport interface with the generic payload and simple sockets
// Shows nb_transport used with the forward and backward paths
// Both components are able to accept transactions on the return path,
// although neither component actually uses the return path (TLM_UPDATED)

// Shows the Approximately Timed coding style
// Models processing delay of initiator, latency of target, and request and response accept delays
// Uses payload event queues to manage both timing annotations and internal delays

// Shows the BEGIN_REQ exclusion rule at the initiator and BEGIN_RESP exclusion rule at the target
// In this example, the target allows two pipelined transactions in-flight

// Shows an explicit memory manager and reference counting

// No use of temporal decoupling, DMI or debug transport
// Nominal use of the blocking transport interface just to show the simple socket b/nb adapter


// Needed for the simple_target_socket
#ifndef __COMMUNICATION_HPP__
#define __COMMUNICATION_HPP__

#define SC_INCLUDE_DYNAMIC_PROCESSES

#include <queue>
#include <systemc>

using namespace sc_core;
using namespace sc_dt;
using namespace std;

#define BUS_WIDTH 16
#define ADDRESS_WIDTH 3
#define NUMBER_OF_ADDRESSES 2 << ADDRESS_WIDTH
#define BUS_DELAY 10 /* FIXME */

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

//#define DEBUG

/* Generic Payload Extension for ID */
struct ID_extension: tlm::tlm_extension<ID_extension> {
  ID_extension() : transaction_id(0) {}
  virtual tlm_extension_base* clone() const { // Must override pure virtual clone method
    ID_extension* t = new ID_extension;
    t->transaction_id = this->transaction_id;
    return t;
  }
  virtual void copy_from(tlm_extension_base const &ext) {
    transaction_id = static_cast<ID_extension const &>(ext).transaction_id;
  }
  unsigned int transaction_id;
};

/* Generic dynamic list */
typedef struct queue{
  unsigned short * datum;
  sc_uint<ADDRESS_WIDTH> address;
  bool cmd;
} queue_element;

static ID_extension* id_extension = new ID_extension;

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

  void write(int adr, int datum, bool cmd) {

    queue_element new_transaction;
    new_transaction.address = adr;
    new_transaction.cmd = cmd;
    new_transaction.datum = new unsigned short;
    *(new_transaction.datum) = datum;

    initiator_queue.push(new_transaction);


    write_req.notify();
  }

  void thread_process()
  {
    tlm::tlm_phase phase;
    sc_time delay = sc_time(BUS_DELAY, SC_NS);;
    

    while(true) {
      wait(write_req);
      
      while(!initiator_queue.empty()) {
        /* Create new transaction */
        tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
        trans->set_extension( id_extension );

        /* Unqueue next transaction */
        queue_element next_transaction = initiator_queue.front();
        initiator_queue.pop();
        int adr = next_transaction.address;
        unsigned short * data = next_transaction.datum;
        tlm::tlm_command cmd = 
            static_cast<tlm::tlm_command>(next_transaction.cmd);
      
        /* Build the transaction */
        trans->set_command( cmd );
        trans->set_address( adr );
        trans->set_data_ptr( reinterpret_cast<unsigned char*>(data) );
        trans->set_data_length( BUS_WIDTH/8 );
        trans->set_streaming_width( BUS_WIDTH/8 ); 
        trans->set_byte_enable_ptr( 0 ); 
        trans->set_dmi_allowed( false );
        trans->set_response_status( tlm::TLM_INCOMPLETE_RESPONSE );

        /* Blocking call */
        socket->b_transport( *trans, delay );

        /* Check response */
        if ( trans->is_response_error() )
          SC_REPORT_ERROR("TLM-2", "Response error from b_transport");

        /* Bus delay for next transaction */
        wait(delay);
        id_extension->transaction_id++;
      }

    }
  }
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

  virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
  {
    tlm::tlm_command tlm_cmd = trans.get_command();
    sc_uint<ADDRESS_WIDTH> adr = trans.get_address();
    unsigned char* ptr = trans.get_data_ptr();
    unsigned int len = trans.get_data_length();
    unsigned char* byt = trans.get_byte_enable_ptr();
    unsigned int wid = trans.get_streaming_width();
    ID_extension* id_extension_tlm = new ID_extension;
    trans.get_extension( id_extension_tlm ); 

    /* Check */
    if (byt != 0 || len > 4 || wid < len)
      SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction");

    /* Write back */
    incoming_buffer = (*reinterpret_cast<int*>(ptr));
    destination_address = adr;
    transfer_package = (adr != module_address);
    command = (tlm_cmd == tlm::TLM_WRITE_COMMAND); /* Write = 1 */
    id_extension = id_extension_tlm->transaction_id;

    /* Alert about new package */
    new_package.notify();
    free(ptr);

    /* Complete */
    trans.set_response_status( tlm::TLM_OK_RESPONSE );

    /* FIXME */
    /* trans->release(); */
  }
};

#endif
