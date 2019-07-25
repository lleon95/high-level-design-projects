#ifndef _IMAGE_PROCESSOR_HPP_
#define _IMAGE_PROCESSOR_HPP_

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#define PIXEL_WIDTH 12
#define CHANNEL_WIDTH 4

#define INTERRUPT_DELAY 0

#define WIDTH 640
#define BUFFER_SIZE (WIDTH * 3)

#define PACKAGE_LENGTH 2 /* 16 bits package length */


struct image_processor : sc_module
{
    tlm_utils::simple_target_socket<image_processor> target_socket;
    tlm_utils::simple_initiator_socket<image_processor> initiator_socket;
   
    sc_event _pixel_ready;

    /* Local Variables */
    sc_uint<PIXEL_WIDTH> current_pixel;
    
    sc_uint<CHANNEL_WIDTH> gray;

    /* Store 3 lines in the internal buffer */
    sc_uint<CHANNEL_WIDTH> pixel_buffer[BUFFER_SIZE];
    int pixel_index;

    /* Interrupt Handler */
    void pixel_ready();

    void process();

    void return_pixel();

  /* Function to receive transfers  */
  virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);

  SC_CTOR(image_processor) : target_socket("target_socket") {
        SC_THREAD(process);
	
	target_socket.register_b_transport(this, &image_processor::b_transport);

	pixel_index = 0;
    } /* End of Constructor */

}; /* End of Module counter */

#endif /* _IMAGE_PROCESSOR_HPP_ */
