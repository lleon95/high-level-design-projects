/* This code includes: a router and a dummy node */

#include "systemc.h"

#include "communication.hpp"

SC_MODULE(Node)
{
  Initiator *initiator;
  Target    *target;
  int addr;

  sc_signal<sc_uint<BUS_WIDTH> > incoming_buffer;
  sc_signal<sc_uint<ADDRESS_WIDTH> > target_address;
  sc_signal<sc_uint<ADDRESS_WIDTH> > initiator_destination_address;
  sc_signal<sc_uint<ADDRESS_WIDTH> > target_destination_address;
  sc_signal<bool > target_transfer_package;
  sc_signal<bool > new_package;

  SC_CTOR(Node)
  {
    // Instantiate components
    initiator = new Initiator("initiator");
    target    = new Target   ("target");

    target->incoming_buffer(incoming_buffer);
    target->destination_address(target_destination_address);
    target->module_address(target_address);
    target->transfer_package(target_transfer_package);
    target->new_package(new_package);

    SC_THREAD(thread_process); 

    /* Method for grabbing new data */
    SC_METHOD(new_package_received);
    sensitive << new_package;
  }

  void thread_process(){
    target_address.write(target_address);
    for(int i = 0; i < 5; i++){
      initiator->addr = i;
      initiator->data = 16 * i;
      wait(sc_time(50, SC_NS));
       initiator->write();
    }
  }

  void new_package_received() {
    bool transfer_next = target_transfer_package.read();
    unsigned short destination = target_destination_address.read();
    unsigned short data = incoming_buffer.read();
    unsigned short my_addr = target_address.read();

    cout << "Destination address: " <<  destination
    << " Me: " << my_addr << " Action - Transfer: "
    << transfer_next << " Data: "
    << incoming_buffer << endl;

    /* Transfer to the next */
    if(transfer_next) {
      initiator->addr = destination;
      initiator->data = data;
      initiator->write();
    } else {
      cout << "Received by: " << my_addr << endl;
    }
  }

};


int sc_main(int argc, char* argv[])
{
  Node node1("node1");
  Node node2("node2");
  Node node3("node3");
  Node node4("node4");
  Node node5("node5");

  /* Address assignment */
  node1.addr = 1;
  node2.addr = 2;
  node3.addr = 3;
  node4.addr = 4;
  node5.addr = 5;

  /* Binding - Ring */
  node1.initiator->socket.bind(node2.target->socket);
  node2.initiator->socket.bind(node3.target->socket);
  node3.initiator->socket.bind(node4.target->socket);
  node4.initiator->socket.bind(node5.target->socket);
  node5.initiator->socket.bind(node1.target->socket);


  sc_start();
  return 0;
}