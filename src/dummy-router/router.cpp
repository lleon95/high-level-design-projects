/* This code includes: a router and a dummy node */

#include "systemc.h"

#include "communication.hpp"

/* Include your nodes below */
#include "dummy-node.cpp"

#define NODES 5

SC_MODULE(Router)
{
  Initiator *initiator_node;
  Initiator *initiator_ring;
  Target    *target_node;
  Target    *target_ring;

  Node      *dummy_node;

  int addr;

  sc_event * incoming_notification_node;
  sc_event * incoming_notification_ring;

  SC_CTOR(Router)
  {
    // Instantiate components
    initiator_node = new Initiator("initiator_node");
    initiator_ring = new Initiator("initiator_ring");
    target_node    = new Target   ("target_node");
    target_ring    = new Target   ("target_ring");
    dummy_node     = new Node     ("dummy_node");

    target_node->module_address = addr;
    target_ring->module_address = addr;

    incoming_notification_node = &(target_node->new_package);
    incoming_notification_ring = &(target_ring->new_package);

    /* Interconnect internals */
    dummy_node->addr = 0; /* Node interconnected to router */
    dummy_node->initiator->socket.bind(target_node->socket);
    initiator_node->socket.bind(dummy_node->target->socket);

    // SC_THREAD(thread_process); 
    SC_THREAD(reading_process_node);
    SC_THREAD(reading_process_ring);

  }

  void reading_process_node() {
    while(true) {
      wait(*(incoming_notification_node));
      bool transfer_next = target_node->transfer_package;
      bool command = target_node->command;
      unsigned short destination = target_node->destination_address;
      unsigned short data = target_node->incoming_buffer;
      wait(sc_time(BUS_DELAY, SC_NS));
      
      cout << "Node-> Destination address: " <<  destination
      << " Me: " << addr << " Action - Transfer: "
      << transfer_next << " Data: "
      << data << " Command: " << command << endl;

      /* Transfer to the next */
      initiator_ring->write(destination, data, command);
      cout << "Retransmitted to: " << addr + 1 << endl;
    }
  }

  void reading_process_ring() {
    while(true) {
      wait(*(incoming_notification_ring));
      bool command = target_ring->command;
      unsigned short destination = target_ring->destination_address;
      unsigned short data = target_ring->incoming_buffer;
      wait(sc_time(BUS_DELAY, SC_NS));
      
      cout << "Ring-> Destination address: " <<  destination
      << " Me: " << addr << " Data: "
      << data << " Command: " << command << endl;

      /* Transfer to the next */
      if(addr == destination) {
        initiator_ring->write(destination, data, command);
        cout << "Retransmitted to: " << addr + 1 << endl;
      } else {
        initiator_node->write(0, data, command); /* 0 is the connected node */
        cout << "Received by: " << addr << endl;
      }
    }
  }
};

int sc_main(int argc, char* argv[])
{
  Router node1("node1");
  Router node2("node2");
  Router node3("node3");
  Router node4("node4");
  Router node5("node5");


  /* Address assignment */
  node1.addr = 0;
  node2.addr = 1;
  node3.addr = 2;
  node4.addr = 3;
  node5.addr = 4;

  /* Binding - Ring */
  node1.initiator_ring->socket.bind(node2.target_ring->socket);
  node2.initiator_ring->socket.bind(node3.target_ring->socket);
  node3.initiator_ring->socket.bind(node4.target_ring->socket);
  node4.initiator_ring->socket.bind(node5.target_ring->socket);
  node5.initiator_ring->socket.bind(node1.target_ring->socket);

  sc_start();
  return 0;
}