#include "systemc.h"

#define WIDTH 640
#define HEIGHT 480
#define PIXEL_IN_WIDTH 12
#define PIXEL_OUT_WIDTH 8

SC_MODULE (image_processor)
{
    sc_in<bool> enable;
    sc_in<sc_uint<PIXEL_IN_WIDTH> > pix_in;
    sc_out<sc_uint<PIXEL_OUT_WIDTH> > pix_out;

    /*------------Local Variables------------------------- */
    sc_uint<4> r;
    sc_uint<4> g;
    sc_uint<4> b;

    /*------------Code Starts Here-------------------------*/
    void convert_to_grayscale () {
        if(enable.read() == 1) {
            r = pix_in.read().range(11, 8);
            g = pix_in.read().range(7, 4);
            b = pix_in.read().range(3, 0);

            pix_out = r * 0.3 + g * 0.59 + b * 0.11;

        }
    }

    SC_CTOR(image_processor) {
        cout<<"Executing new"<<endl;
        SC_METHOD(convert_to_grayscale);
        sensitive << pix_in;
    } /* End of Constructor */

}; /* End of Module counter */
