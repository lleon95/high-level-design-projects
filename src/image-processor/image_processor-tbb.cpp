
#include <math.h>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>

#include "image_processor.hpp"
#define TEST_ITERATIONS 10

/* Definition of Sobel filter in horizontal direction */
const int h_weights[3][3] = {
    { -1,  0,  1 },
    { -2,  0,  2 },
    { -1,  0,  1 }
};

/* Definition of Sobel filter in vertical direction */
const int v_weights[3][3] = {
    { -1,  -2, -1 },
    { 0,  0,  0 },
    { 1,  2,  1 }
};


int
rgb12_to_gray(int pixel)
{
    int r = ((pixel & 0xF00) >> 8) * 0.3;
    int g = ((pixel & 0x0F0) >> 4) * 0.59;
    int b = ((pixel & 0x00F) >> 0) * 0.11;

    int gray = r + g + b;
    gray = gray  % ( 1 << CHANNEL_WIDTH );

    return gray;
}

int
rgb12_to_sobel(int pixels[9])
{
    int h_value = 0;
    int v_value = 0;
    int result;

    for (int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
            h_value += h_weights[i][j] * rgb12_to_gray(pixels[i* 3 + j]);
            v_value += v_weights[i][j] * rgb12_to_gray(pixels[i* 3 + j]);
        }
    }

    /* Get sobel value */
    result =  sqrt(h_value*h_value + v_value*v_value);
    result = result  % ( 1 << CHANNEL_WIDTH );


    return (result << (CHANNEL_WIDTH * 2)) + (result << (CHANNEL_WIDTH * 1)) + result;
}




int
sc_main (int argc, char* argv[])
{
    int random_pixel = 0;
    srand (time(NULL));

    int test_pixels[9];
    sc_signal<bool>   enable;
    sc_signal<sc_uint<PIXEL_WIDTH> > pixels[9];

    sc_signal<sc_uint<PIXEL_WIDTH> > pix_out;

    int i = 0;
    double total_error = 0;

    /* Connect the DUT */
    image_processor processor("GRAYSCALER");
    processor.enable(enable);
    for(int i = 0; i < 9; i++) {
        processor.in_pixels[i](pixels[i]);
    }

    processor.pix_out(pix_out);

    sc_start(1,SC_NS);

    /* Open VCD file */
    sc_trace_file *wf = sc_create_vcd_trace_file("image_processor");

    /* Dump the desired signals */
    sc_trace(wf, enable, "enable");
    sc_trace(wf, pix_out, "pix_out");
    sc_trace(wf, pixels[0], "top_left");
    sc_trace(wf, pixels[1], "top_center");
    sc_trace(wf, pixels[2], "top_right");
    sc_trace(wf, pixels[3], "center_left");
    sc_trace(wf, pixels[4], "center");
    sc_trace(wf, pixels[5], "center_right");
    sc_trace(wf, pixels[6], "bottom_left");
    sc_trace(wf, pixels[7], "bottom_center");
    sc_trace(wf, pixels[8], "bottom_right");

    /* Initialize all variables */
    for (i=0; i<TEST_ITERATIONS; i++) {
        /* Generate random number */
        for (int i = 0; i < 9; i++) {
            random_pixel = rand() % ( 1 << PIXEL_WIDTH );
	    test_pixels[i] = random_pixel;
            pixels[i].write(random_pixel);
        }

        sc_start(1,SC_NS);

        /* Start frame */
        enable.write(1);
        sc_start(1,SC_NS);

        /*
        * Test last pixel, since this is PV all pixels are inmediatly
               * available
               */
        total_error += abs((int) pix_out.read() - rgb12_to_sobel(test_pixels));
        printf("input pixel: %x\tsystemC output = %x\texpected output = %x\n",
               random_pixel, (int)pix_out.read(),  rgb12_to_sobel(test_pixels));


        enable.write(0);
        sc_start(1,SC_NS);
    }

    cout << "Average Error: " << total_error / TEST_ITERATIONS << endl;


    cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
    sc_close_vcd_trace_file(wf);
    return 0; /* Terminate simulation */
}
