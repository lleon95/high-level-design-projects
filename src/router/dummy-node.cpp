#include "dummy-node.hpp"

void
DummyNode::thread_process()
{
    target->module_address = addr;
    for(int i = 0; i < MAX_ADDRESS; i++) {
        wait(sc_time(BUS_DELAY, SC_NS));
        initiator->write(i, rand(), rand() % 2);
    }

}

void
DummyNode::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        bool transfer_next = target->transfer_package;
        bool command = target->command;
        unsigned short destination = target->destination_address;
        unsigned short data = target->incoming_buffer;
        wait(sc_time(BUS_DELAY, SC_NS));

        cout << "Trans ID: " << target->id_extension << " Destination address: " <<
             destination
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
