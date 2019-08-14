#ifndef _ROUTER_
#define _ROUTER_

/* This code includes: a router and a dummy node */

#include "systemc.h"

#include "communication.hpp"

/* FIXME - Include your nodes below */
#include "node.hpp"

SC_MODULE(Router)
{
  Initiator *initiator_node;
  Initiator *initiator_ring;
  Target    *target_node;
  Target    *target_ring;

  Node      *node;

  int addr;

  sc_event * incoming_notification_node;
  sc_event * incoming_notification_ring;

  SC_HAS_PROCESS(Router);

  Router(sc_module_name name_, Node *_node);

  void reading_process_node();

  void reading_process_ring();
};

#endif /* _ROUTER_ */
