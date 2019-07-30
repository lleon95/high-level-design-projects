#ifndef _MEMORY_HPP_
#define _MEMORY_HPP_

#include "systemc.h"
#include "node.hpp"

/* Matches pixel width */
#define DATA_WIDTH 12
/* Number of images to store, in this case an input and an output image */
#define IMAGE_BUFFERS 2
/* Address bus width, simulator give error for bigger values */
#define ADDR_WIDTH 18
/* Pixels in an image ( 640 * 480 * IMAGE_BUFFERS ) */
#define DATA_DEPTH 1 << ADDR_WIDTH


struct memory : Node {

  sc_uint<DATA_WIDTH> _ramdata[DATA_DEPTH];

  void thread_process();
  void reading_process();

  sc_event update_event;
  unsigned short _data;

  SC_HAS_PROCESS(memory);
  memory(const sc_module_name & name) : Node(name)
  { }

}; /* End of memory module */

#endif /* _MEMORY_HPP_ */
