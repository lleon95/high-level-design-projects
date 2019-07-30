#ifndef _DUMMY_NODE_HPP_
#define _DUMMY_NODE_HPP_

#include "node.hpp"

#define DUMMY_PACKAGES 5

struct DummyNode : public Node
{

  sc_event update_event;
  /* Initialization done by the parent class */
  DummyNode(const sc_module_name & name) : Node(name) {
  }

  void thread_process();

  void reading_process();
};

#endif /* _DUMMY_NODE_HPP_ */








