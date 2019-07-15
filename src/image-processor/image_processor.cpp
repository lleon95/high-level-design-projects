#include "systemc.h"

#include <math.h>

//#define WIDTH 640
//#define HEIGHT 480

#define WIDTH 4
#define HEIGHT 4


#define PIXEL_WIDTH 12
#define CHANNEL_WIDTH 4

#define ADDRESS_WIDTH 20
#define INPUT_IMAGE_START 0x0
#define OUTPUT_IMAGE_START 0x4b000

#define INTERRUPT_DELAY 0

/* Definition of Sobel filter in horizontal direction */
const sc_int<CHANNEL_WIDTH+1> h_weights[3][3] = {
    { -1,  0,  1 },
    { -2,  0,  2 },
    { -1,  0,  1 }
};

/* Definition of Sobel filter in vertical direction */
const sc_int<CHANNEL_WIDTH+1> v_weights[3][3] = {
    { -1,  -2, -1 },
    { 0,  0,  0 },
    { 1,  2,  1 }
};


SC_MODULE (image_processor)
{
    sc_in<bool> enable;
    sc_inout<sc_uint<PIXEL_WIDTH> > pix_data;
    sc_inout<sc_uint<PIXEL_WIDTH> > pix_data_out;
    
    sc_out<sc_uint<ADDRESS_WIDTH> > pix_address;

    sc_event _frame_start;

    /* Local Variables */
    sc_uint<PIXEL_WIDTH> current_pixel;

    sc_uint<CHANNEL_WIDTH> r;
    sc_uint<CHANNEL_WIDTH> g;
    sc_uint<CHANNEL_WIDTH> b;
    sc_uint<CHANNEL_WIDTH> gray;

    sc_uint<CHANNEL_WIDTH> pixels[3][3];

    /* Interrupt Handler */
    void frame_start() {
        _frame_start.notify(INTERRUPT_DELAY, SC_NS);
    }

    sc_uint<CHANNEL_WIDTH> apply_sobel () {
        sc_int<2*CHANNEL_WIDTH> h_value = 0;
        sc_int<2*CHANNEL_WIDTH> v_value = 0;

        /* Get horizontal values */
        for (int i = 0; i < 3; i++) {
            for( int j = 0; j < 3; j++) {
                h_value += h_weights[i][j] * pixels[i][j];
            }
        }

        /* Get vertical values */
        for (int i = 0; i < 3; i++) {
            for( int j = 0; j < 3; j++) {
                v_value += v_weights[i][j] * pixels[i][j];
            }
        }

        /* Get sobel value */
        return sqrt(h_value*h_value + v_value*v_value);

    }

    sc_uint<CHANNEL_WIDTH> convert_to_grayscale (sc_uint<PIXEL_WIDTH> pixel) {
        /* Get component values from memory */
        r = pixel.range(3*CHANNEL_WIDTH - 1,
                        2*CHANNEL_WIDTH) * 0.3;
        g = pixel.range(2*CHANNEL_WIDTH - 1,
                        1*CHANNEL_WIDTH) * 0.59;
        b = pixel.range(1*CHANNEL_WIDTH - 1,
                         0) * 0.11;

        /* Get grayscale value */
        gray =  r + g + b;

        return gray;
    }

    void process() {
        while(true) {
            wait(_frame_start);

	    /* Operate over all pixels */
            for(int i = 0; i < HEIGHT; i++) {
                for(int j = 0; j < WIDTH; j++) {
		  
                    /* If this is the first pixel copy the values to the interal buffers */
                    if( (i == 0) && (j == 0) ) {
                        for (int k = 0; k < 3; k++) {
                            for( int l = 0; l < 3; l++) {
                                pix_address = INPUT_IMAGE_START + j + l  + (i +k ) * WIDTH;
                                pixels[k][l] = convert_to_grayscale (pix_data.read());
                            }
                        }
                    } else {
                        /* Update the internal values from the old values */
                        for (int k = 0; k < 3; k++) {
                            for( int l = 0; l < 2; l++) {
			      pixels[k][l] = (int)pixels[k][l+1];
                            }
                        }

                        /* Get new values from the memory */
                        for (int k = 0; k < 3; k++) {
			  pix_address = INPUT_IMAGE_START + (j + 2) + (i+k) * WIDTH;
                            pixels[k][2] = convert_to_grayscale (pix_data.read());
                        }
                    }

                    /* Apply sobel if not on the border */
                    if( (i != 0) && ( i != (HEIGHT - 1)) && (j != 0) && (j != (WIDTH -1))) {
                        gray = apply_sobel();
                    }
                    /* Otherwise use passthrough */
                    else {
		      gray = pixels[1][1];
                    }

		    /* Get values to output pixel */
		    current_pixel.range(3*CHANNEL_WIDTH - 1, 2*CHANNEL_WIDTH) = gray;
		    current_pixel.range(2*CHANNEL_WIDTH - 1, 1*CHANNEL_WIDTH) = gray;
		    current_pixel.range(1*CHANNEL_WIDTH - 1, 0) = gray;

                    /* Write value back*/
                    pix_address = OUTPUT_IMAGE_START + j + i * WIDTH;
                    pix_data_out.write(current_pixel);
                }
            }
        }
    }

    SC_CTOR(image_processor) {
        cout<<"Executing new"<<endl;
        SC_THREAD(process);

        SC_METHOD(frame_start);
        sensitive << enable;
    } /* End of Constructor */

}; /* End of Module counter */
