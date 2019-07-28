#ifndef _NODE_HPP_
#define _NODE_HPP_

#include "systemc.h"

#include "communication.hpp"

#define NODES 5

SC_MODULE(Node)
{
    Initiator *initiator;
    Target    *target;
    int addr;
    sc_event * incoming_notification;

    SC_HAS_PROCESS(Node);

    Node(const sc_module_name& name) : sc_module(name) {
        // Instantiate components
        initiator = new Initiator("initiator");
        target    = new Target   ("target");

        incoming_notification = &(target->new_package);

        SC_THREAD(thread_process);
        SC_THREAD(reading_process);
    }

    virtual void thread_process() = 0;

    virtual void reading_process() = 0;
};

#endif /* _NODE_HPP_ */
