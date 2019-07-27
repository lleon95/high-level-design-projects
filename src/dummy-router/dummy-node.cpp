/* This code includes: a router and a dummy node */

#include "systemc.h"

#include "communication.hpp"

#define NODES 5

SC_MODULE(Node)
{
  Initiator *initiator;
  Target    *target;
  int addr;
  sc_event * incoming_notification;

  SC_CTOR(Node)
  {
    // Instantiate components
    initiator = new Initiator("initiator");
    target    = new Target   ("target");

    incoming_notification = &(target->new_package);

    SC_THREAD(thread_process); 
    SC_THREAD(reading_process);
  }

  void thread_process(){
    target->module_address = addr;
    for(int i = 0; i < NODES; i++){
      wait(sc_time(BUS_DELAY, SC_NS));
      initiator->write(i, 16 * i + 1, rand() % 2);
    }
  }

  void reading_process() {
    while(true) {
      wait(*(incoming_notification));
      bool transfer_next = target->transfer_package;
      bool command = target->command;
      unsigned short destination = target->destination_address;
      unsigned short data = target->incoming_buffer;
      wait(sc_time(BUS_DELAY, SC_NS));
      
      cout << "Trans ID: " << target->id_extension << " Destination address: " <<  destination
      << " Me: " << addr << " Action - Transfer: "
      << transfer_next << " Data: "
      << data << " Command: " << command << endl;

      /* Transfer to the next */
      if(transfer_next) {
        initiator->write(destination, data, command);
        cout << "Retransmitted to: " << addr + 1 << endl;
      } else {
        cout << "Received by: " << addr << endl;
      }
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
  node1.addr = 0;
  node2.addr = 1;
  node3.addr = 2;
  node4.addr = 3;
  node5.addr = 4;

  /* Binding - Ring */
  node1.initiator->socket.bind(node2.target->socket);
  node2.initiator->socket.bind(node3.target->socket);
  node3.initiator->socket.bind(node4.target->socket);
  node4.initiator->socket.bind(node5.target->socket);
  node5.initiator->socket.bind(node1.target->socket);

  sc_start();
  return 0;
}