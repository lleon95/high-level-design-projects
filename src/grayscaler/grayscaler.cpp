#include "systemc.h"

#define WIDTH 640
#define HEIGHT 480
#define PIXEL_WIDTH 12
#define CHANNEL_WIDTH 4

#define ADDRESS_WIDTH 20
#define COLOR_IMAGE_START 0x0
#define GRAY_IMAGE_START 0x4b000

#define INTERRUPT_DELAY 0

SC_MODULE (grayscaler)
{
    sc_in<bool> enable;
    sc_inout<sc_uint<PIXEL_WIDTH> > pix_data;
    sc_out<sc_uint<ADDRESS_WIDTH> > pix_address;

    sc_event _frame_start;

    /* Local Variables */
    sc_uint<PIXEL_WIDTH> current_pixel;
    sc_uint<2*CHANNEL_WIDTH> r;
    sc_uint<2*CHANNEL_WIDTH> g;
    sc_uint<2*CHANNEL_WIDTH> b;
    sc_uint<CHANNEL_WIDTH> gray;

    /* Interrupt Handler */
    void frame_start() {
        _frame_start.notify(INTERRUPT_DELAY, SC_NS);
    }


    void convert_to_grayscale () {
        while(true) {
            wait(_frame_start);
            for(int i = 0; i < HEIGHT; i++) {
                for(int j = 0; j < WIDTH; j++) {
                    /* Copy value from memory */
                    pix_address = COLOR_IMAGE_START + j + i * WIDTH;

                    current_pixel = pix_data.read();

                    /* Get component values from memory */

                    r = current_pixel.range(3*CHANNEL_WIDTH - 1,
                                            2*CHANNEL_WIDTH) * 0.3;
                    g = current_pixel.range(2*CHANNEL_WIDTH - 1,
                                            1*CHANNEL_WIDTH) * 0.59;
                    b = current_pixel.range(1*CHANNEL_WIDTH - 1,
                                            0) * 0.11;

                    /* Get grayscale value */
                    gray =  r + g + b;

                    /* Get values to output pixel */
                    current_pixel.range(3*CHANNEL_WIDTH - 1,
                                        2*CHANNEL_WIDTH) = gray;
                    current_pixel.range(2*CHANNEL_WIDTH - 1,
                                        1*CHANNEL_WIDTH) = gray;
                    current_pixel.range(1*CHANNEL_WIDTH - 1,
                                        0) = gray;

                    /* Write new value back to memory  */
                    pix_address = GRAY_IMAGE_START + j + i * WIDTH;
                    pix_data.write(current_pixel);

                }
            }
        }
    }

    SC_CTOR(grayscaler) {
        cout<<"Executing new"<<endl;
        SC_THREAD(convert_to_grayscale);

        SC_METHOD(frame_start);
        sensitive << enable;
    } /* End of Constructor */

}; /* End of Module counter */
