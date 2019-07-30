/* This code includes: a router and a dummy node */

#include "router.hpp"

Router::Router(sc_module_name name_, Node* _node)  : sc_module(name_)
{
    node = _node;
    /* Instantiate components */
    initiator_node = new Initiator("initiator_node");
    initiator_ring = new Initiator("initiator_ring");
    target_node    = new Target   ("target_node");
    target_ring    = new Target   ("target_ring");

    target_node->module_address = addr;
    target_ring->module_address = addr;

    incoming_notification_node = &(target_node->new_package);
    incoming_notification_ring = &(target_ring->new_package);

    /* Interconnect internals */
    node->addr = 0; /* Node interconnected to router */
    node->initiator->socket.bind(target_node->socket);
    initiator_node->socket.bind(node->target->socket);

    SC_THREAD(reading_process_node);
    SC_THREAD(reading_process_ring);
}

static int global_id_counter = 0;


void
Router::reading_process_node()
{
    while(true) {
        wait(*(incoming_notification_node));
        bool transfer_next = target_node->transfer_package;
        bool command = target_node->command;
        unsigned short destination = target_node->destination_address;
        unsigned short data = target_node->incoming_buffer;
	int id = global_id_counter++;
        wait(sc_time(BUS_DELAY, SC_NS));

	cout << "ID: " << id << "\t";

        cout << "Node-> Destination address: " <<  destination
             << " Me: " << addr << " Action - Transfer: "
             << transfer_next << " Data: "
             << data << " Command: " << command << endl;

        /* Transfer to the next */
        initiator_ring->write(destination, data, command, id);
        cout << "Retransmitted to: " << addr + 1 << endl;
    }
}

void
Router::reading_process_ring()
{
    while(true) {
        wait(*(incoming_notification_ring));
        bool command = target_ring->command;
        unsigned short destination = target_ring->destination_address;
        unsigned short data = target_ring->incoming_buffer;
	int id = target_ring->id_extension;
        wait(sc_time(BUS_DELAY, SC_NS));

	cout << "ID: " << id << "\t";
	
        cout << "Ring-> Destination address: " <<  destination
             << " Me: " << addr << " Data: "
             << data << " Command: " << command << endl;

        /* Transfer to the next */
        if(addr != destination) {
	  initiator_ring->write(destination, data, command, id);
            cout << "Retransmitted to: " << (addr + 1) % (DAC_ADDRESS+1)  << endl;
        } else {
	  initiator_node->write(0, data, command, id); /* 0 is the connected node */
            cout << "Received by: " << addr << endl;
        }
    }
}
