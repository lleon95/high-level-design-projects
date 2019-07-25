/* This code includes: a router and a dummy node */

#include "systemc.h"

#include "communication.hpp"

SC_MODULE(Top)   
{   
  Initiator *initiator;   
  Target    *target;   

  sc_signal<sc_uint<BUS_WIDTH> > incoming_buffer;
  sc_signal<sc_uint<BUS_WIDTH> > outgoing_buffer;
  sc_signal<sc_uint<ADDRESS_WIDTH> > target_address;
  sc_signal<sc_uint<ADDRESS_WIDTH> > initiator_address;
  sc_signal<sc_uint<ADDRESS_WIDTH> > initiator_destination_address;
  sc_signal<sc_uint<ADDRESS_WIDTH> > target_destination_address;
  sc_signal<bool > target_transfer_package;
  sc_signal<bool > new_package;

  SC_CTOR(Top)   
  {   
    // Instantiate components   
    initiator = new Initiator("initiator");   
    target    = new Target   ("target");   
   
    // One initiator is bound directly to one target with no intervening bus   
   
    // Bind initiator socket to target socket   
    initiator->socket.bind(target->socket); 

    /* Connect pins */
    initiator->outgoing_buffer(outgoing_buffer);
    initiator->destination_address(initiator_destination_address);
    initiator->module_address(initiator_address);

    target->incoming_buffer(incoming_buffer);
    target->destination_address(target_destination_address);
    target->module_address(target_address);
    target->transfer_package(target_transfer_package);
    target->new_package(new_package);

    SC_THREAD(thread_process);  
  }   

  void thread_process(){
    initiator_address.write(5);
    target_address.write(1);
    outgoing_buffer.write(20);
    initiator_destination_address.write(1);
    initiator->write(25);

    wait(sc_time(BUS_DELAY, SC_NS));

    outgoing_buffer.write(10);
    initiator_destination_address.write(0);
    initiator->write();
  }

};   
   
   
int sc_main(int argc, char* argv[])   
{   
  Top top("top");   
  sc_start();   
  return 0;   
}   
