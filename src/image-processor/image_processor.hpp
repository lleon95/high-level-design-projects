#ifndef _IMAGE_PROCESSOR_HPP_
#define _IMAGE_PROCESSOR_HPP_

#include "systemc.h"

#define PIXEL_WIDTH 12
#define CHANNEL_WIDTH 4

#define ADDRESS_WIDTH 20
#define INPUT_IMAGE_START 0x0
#define OUTPUT_IMAGE_START 0x4b000

#define INTERRUPT_DELAY 0


SC_MODULE (image_processor)
{
    sc_in<bool> enable;
    sc_in<sc_uint<PIXEL_WIDTH> > in_pixels[9];
    sc_out<sc_uint<PIXEL_WIDTH> > pix_out;

    sc_event _frame_start;

    /* Local Variables */
    sc_uint<PIXEL_WIDTH> current_pixel;
    
    sc_uint<CHANNEL_WIDTH> gray;

    sc_uint<CHANNEL_WIDTH> pixels[3][3];

    /* Interrupt Handler */
    void frame_start();

    void process();

    SC_CTOR(image_processor) {
        cout<<"Executing new"<<endl;
        SC_THREAD(process);

        SC_METHOD(frame_start);
        sensitive << enable;
    } /* End of Constructor */

}; /* End of Module counter */

#endif /* _IMAGE_PROCESSOR_HPP_ */
