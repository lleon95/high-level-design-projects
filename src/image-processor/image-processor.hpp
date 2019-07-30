#ifndef _IMAGE_PROCESSOR_HPP_
#define _IMAGE_PROCESSOR_HPP_

#include "systemc.h"
#include "tlm.h"
#include "node.hpp"

#define PIXEL_WIDTH 12
#define CHANNEL_WIDTH 4

#define INTERRUPT_DELAY 0

#define WIDTH 640
#define HEIGHT 480
#define BUFFER_SIZE (WIDTH * 3)

#define PACKAGE_LENGTH 2 /* 16 bits package length */


struct image_processor : Node
{ 
    sc_event _pixel_ready;

    /* Local Variables */
    sc_uint<PIXEL_WIDTH> current_pixel;
    
    sc_uint<CHANNEL_WIDTH> gray;

    /* Store 3 lines in the internal buffer */
    sc_uint<CHANNEL_WIDTH> pixel_buffer[BUFFER_SIZE];
    int pixel_index;

    void thread_process();

    void reading_process();


  image_processor(const sc_module_name & name) : Node(name) {
	pixel_index = 0;

	for(int i = 0; i < BUFFER_SIZE; i++) {
	  pixel_buffer[i] = 0;
	}
    } /* End of Constructor */

}; /* End of Module counter */

#endif /* _IMAGE_PROCESSOR_HPP_ */
