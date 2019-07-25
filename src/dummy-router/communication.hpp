/*
  This file has the router model and interconnections between modules

  Router: it is the router model
*/
#ifndef __COMMUNICATION_HPP__
#define __COMMUNICATION_HPP__

#define DEBUG
#include <systemc.h>   

#define BUS_WIDTH 16
#define ADDRESS_WIDTH 3
#define NUMBER_OF_ADDRESSES 2 << ADDRESS_WIDTH
#define BUS_DELAY 10 /* FIXME */

using namespace sc_core;   
using namespace sc_dt;   
using namespace std;   
   
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/peq_with_cb_and_phase.h"

// User-defined extension class
struct ID_extension: tlm::tlm_extension<ID_extension> {
  ID_extension() : transaction_id(0) {}
  virtual tlm_extension_base* clone() const { // Must override pure virtual clone method
    ID_extension* t = new ID_extension;
    t->transaction_id = this->transaction_id;
    return t;
  }

  // Must override pure virtual copy_from method
  virtual void copy_from(tlm_extension_base const &ext) {
    transaction_id = static_cast<ID_extension const &>(ext).transaction_id;
  }
  unsigned int transaction_id;
};



// Initiator module generating generic payload transactions   
struct Initiator: sc_module   
{   
  sc_in<sc_uint<BUS_WIDTH> > outgoing_buffer;
  sc_in<sc_uint<ADDRESS_WIDTH> > module_address;
  sc_in<sc_uint<ADDRESS_WIDTH> > destination_address;
  int data; 
  
  tlm_utils::simple_initiator_socket<Initiator> socket; 
  sc_event write_event;
   
  SC_CTOR(Initiator)   
  : socket("socket")    
  {   
    socket.register_nb_transport_bw(this, &Initiator::nb_transport_bw);
    SC_THREAD(thread_process);   
  }  

  void write(data_in) {
    data = data_in;
    write_event.notify();
  } 
   
  void thread_process()   
  {   
    
    ID_extension* id_extension = new ID_extension;
    while(true) {
      tlm::tlm_generic_payload trans;
      wait(write_event);
      /* Create a new transaction */
      trans.set_extension( id_extension );

      tlm::tlm_phase phase = tlm::BEGIN_REQ;   
      sc_time delay = sc_time(BUS_DELAY, SC_NS); /* FIXME */
  
      trans.set_command( tlm::TLM_WRITE_COMMAND );   
      trans.set_address( destination_address.read() );  
      data = outgoing_buffer.read(); /* FIXME */
      trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));   
      trans.set_data_length( BUS_WIDTH/8 );   
   
      /* Delay for Begin Req */
      wait( sc_time(BUS_DELAY, SC_NS) );
      tlm::tlm_sync_enum status;

      cout << "Sending -> Address: " << destination_address.read() << " Data: " << data << endl;

      /* Send new request */
      cout << name() << " BEGIN_REQ SENT" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      status = socket->nb_transport_fw( trans, phase, delay );  // Non-blocking transport call   
   
      /* Check returned */
      switch (status)   
      {   
      case tlm::TLM_ACCEPTED:   
        
        //Delay for END_REQ
        wait( sc_time(BUS_DELAY, SC_NS) );
        
        cout << name() << " END_REQ SENT" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
        // Expect response on the backward path  
        phase = tlm::END_REQ; 
        status = socket->nb_transport_fw( trans, phase, delay );  // Non-blocking transport call
        break;   
   
      case tlm::TLM_UPDATED:   
      case tlm::TLM_COMPLETED:   
   
        // Initiator obliged to check response status   
        if (trans.is_response_error() )   
          SC_REPORT_ERROR("TLM2", "Response error from nb_transport_fw");   
   
        break;   
      }
    }
    
    /* Delay for RD/WR Reqs */
    wait(BUS_DELAY, SC_NS);   
    id_extension->transaction_id++; 
  }   
   
  virtual tlm::tlm_sync_enum nb_transport_bw( tlm::tlm_generic_payload& trans,   
                                           tlm::tlm_phase& phase, sc_time& delay )   
  {   
    tlm::tlm_command cmd = trans.get_command();   
    sc_dt::uint64    adr = trans.get_address();   
    
    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension ); 

    switch (phase)
    {
    case tlm::END_RESP:
      wait(delay);
      cout << name() << " END_RESP RECEIVED" << " at time " << sc_time_stamp() << endl;
      return tlm::TLM_COMPLETED;
      break;
    case tlm::BEGIN_RESP:
      if (trans.is_response_error() )   
        SC_REPORT_ERROR("TLM2", "Response error from nb_transport");  
      wait(delay);
      cout << name () << " BEGIN_RESP RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      return tlm::TLM_ACCEPTED;   
      break;
    default:
      break;
    }
  }    
  
};   
   
   
// Target module representing a simple memory   
struct Target: sc_module   
{   
  sc_out<sc_uint<BUS_WIDTH> > incoming_buffer;
  sc_in<sc_uint<ADDRESS_WIDTH> > module_address;
  sc_out<sc_uint<ADDRESS_WIDTH> > destination_address;
  sc_out<bool > transfer_package;
  sc_out<bool > new_package;

  // TLM-2 socket, defaults to 32-bits wide, base protocol
  tlm_utils::simple_target_socket<Target> socket;
  
  enum { SIZE = 256 };   
  const sc_time LATENCY;   
   
  SC_CTOR(Target)   
  : socket("socket"), LATENCY(BUS_DELAY, SC_NS)   
  {   
    // Register callbacks for incoming interface method calls
    socket.register_nb_transport_fw(this, &Target::nb_transport_fw);
    SC_THREAD(thread_process);   
  }   
   
  // TLM2 non-blocking transport method 
  virtual tlm::tlm_sync_enum nb_transport_fw( tlm::tlm_generic_payload& trans,
                                              tlm::tlm_phase& phase, sc_time& delay )
  {
    sc_dt::uint64    adr = trans.get_address();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    ID_extension* id_extension = new ID_extension;
    trans.get_extension( id_extension ); 

    switch (phase)
    {
    case tlm::END_REQ:
      wait(delay);
      cout << name() << " END_REQ RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;
      return tlm::TLM_COMPLETED;
      break;
    
    case tlm::BEGIN_REQ:
      if (byt != 0) {
        trans.set_response_status( tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE );
        return tlm::TLM_COMPLETED;
      }
      trans_pending=&trans;
      phase_pending=phase;
      delay_pending=delay;
      e1.notify();
      wait(delay);
      cout << name() << " BEGIN_REQ RECEIVED" << " TRANS ID " << id_extension->transaction_id << " at time " << sc_time_stamp() << endl;      
      return tlm::TLM_ACCEPTED;
      break;
    
    default:
      break;
    } 
  }
  
  void thread_process()  
  {   
    while (true) {
    
      // Wait for an event to pop out of the back end of the queue   
      wait(e1); 
  
      tlm::tlm_phase phase;   
      
      ID_extension* id_extension = new ID_extension;
      trans_pending->get_extension( id_extension ); 
      
      tlm::tlm_command cmd = trans_pending->get_command();   
      sc_dt::uint64    adr = trans_pending->get_address() / 4;   
      unsigned char*   ptr = trans_pending->get_data_ptr();   
      unsigned int     len = trans_pending->get_data_length();   
      unsigned char*   byt = trans_pending->get_byte_enable_ptr();   
      unsigned int     wid = trans_pending->get_streaming_width();   
   
      if (adr >= sc_dt::uint64(SIZE) || byt != 0 || wid != 0 || len > 4)   
        SC_REPORT_ERROR("TLM2", "Target does not support given generic payload transaction");   
      
      /* Perform logic here */ 
      destination_address.write(adr);
      transfer_package = (module_address.read() == adr); /* It is the final destination */
      incoming_buffer.write(*(ptr));
      new_package.write(1);
      wait( sc_time(1, SC_NS) );
      new_package.write(0);

      cout << "Receiving("<< module_address.read() <<") -> Address: " << adr << " Data: " << *(ptr) << endl;
             
      // Obliged to set response status to indicate successful completion   
      trans_pending->set_response_status( tlm::TLM_OK_RESPONSE );  
      
      wait( sc_time(BUS_DELAY, SC_NS) );
      delay_pending= sc_time(BUS_DELAY, SC_NS);
      cout << name() << " BEGIN_RESP SENT" << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
      
      // Call on backward path to complete the transaction
      tlm::tlm_sync_enum status;
        phase = tlm::BEGIN_RESP;   
      status = socket->nb_transport_bw( *trans_pending, phase, delay_pending );   
   
      // Check value returned from nb_transport   
      switch (status)     
      case tlm::TLM_ACCEPTED:   
        wait( sc_time(BUS_DELAY, SC_NS) );
        cout << name() << " END_RESP SENT" << " TRANS ID " << id_extension->transaction_id <<  " at time " << sc_time_stamp() << endl;
        // Expect response on the backward path  
        phase = tlm::END_RESP; 
        socket->nb_transport_bw( *trans_pending, phase, delay_pending );  // Non-blocking transport call
    }   
  } 
   
  sc_event  e1;
  tlm::tlm_generic_payload* trans_pending;   
  tlm::tlm_phase phase_pending;   
  sc_time delay_pending;
    
};   

#endif
