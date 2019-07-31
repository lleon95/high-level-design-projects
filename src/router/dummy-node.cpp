#include "dummy-node.hpp"

void
DummyNode::thread_process()
{

    for(int i = 0; i < DUMMY_PACKAGES; i++) {
        unsigned short data = 0xdf & i + 0xd0;
        initiator->write(7, data, rand() % 2);
        cout << "Dummy write:\t" << data << " @ " << sc_time_stamp() << endl;
        wait(update_event);
    }

}

void
DummyNode::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        
        bool cmd = target->command;
        unsigned short data = target->incoming_buffer;

        if (cmd == tlm::TLM_WRITE_COMMAND) {
            cout << "Dummy received:\t" << data << " @ " << sc_time_stamp() << endl;
        }

        update_event.notify();
    }
}
