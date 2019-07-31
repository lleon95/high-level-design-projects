#include "communication.hpp"

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
  unsigned int transaction_id;
  ID_extension(int id) : transaction_id(0) {
    this->transaction_id = id;
  }
  ID_extension() : transaction_id(0) {}
  virtual tlm_extension_base* clone() const { // Must override pure virtual clone method
    ID_extension* t = new ID_extension;
    t->transaction_id = this->transaction_id;
    return t;
  }
  virtual void copy_from(tlm_extension_base const &ext) {
    transaction_id = static_cast<ID_extension const &>(ext).transaction_id;
  }
};

static ID_extension* id_extension = new ID_extension;

/*
  Initiator
 */

void Initiator::write(int adr, int datum, bool cmd, int old_id) {
    queue_element new_transaction;
    new_transaction.address = adr;
    new_transaction.cmd = cmd;
    new_transaction.datum = new unsigned short;
    *(new_transaction.datum) = datum;
    new_transaction.id = old_id;

    initiator_queue.push(new_transaction);

    write_req.notify();
  }

void Initiator::thread_process()
  {
    tlm::tlm_phase phase;
    sc_time delay = sc_time(BUS_DELAY, SC_NS);;
    

    while(true) {
      wait(write_req);
      
      while(!initiator_queue.empty()) {
        /* Create new transaction */
        tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;

        /* Unqueue next transaction */
        queue_element next_transaction = initiator_queue.front();
        initiator_queue.pop();
        int adr = next_transaction.address;
        unsigned short * data = next_transaction.datum;
        tlm::tlm_command cmd = 
            static_cast<tlm::tlm_command>(next_transaction.cmd);
	int id = next_transaction.id;

	ID_extension* id_extension_tlm = new ID_extension(id);
	trans->set_extension( id_extension_tlm );
	        
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

/*
  Target
 */
void Target::b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
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
