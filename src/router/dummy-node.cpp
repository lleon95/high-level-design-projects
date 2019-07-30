#include "dummy-node.hpp"

void
DummyNode::thread_process()
{

    for(int i = 0; i < DUMMY_PACKAGES; i++) {
        wait(update_event);
        initiator->write(i, 0xdf & rand() % 15, rand() % 2);
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

        update_event.notify
    }
}
