
#include <math.h>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>

#include "image_processor.cpp"

#define TEST_ITERATIONS 20

/* Get the exact expected value */
int
rgb12_to_gray(int pixel)
{
    int r = ((pixel & 0xF00) >> 8) * 0.3;
    int g = ((pixel & 0x0F0) >> 4) * 0.59;
    int b = ((pixel & 0x00F) >> 0) * 0.11;

    int gray = r + g + b;

    return (gray << (CHANNEL_WIDTH * 2)) + (gray << (CHANNEL_WIDTH * 1)) + gray;
}

int
sc_main (int argc, char* argv[])
{
    int random_pixel = 0;
    srand (time(NULL));

    sc_signal<bool>   enable;
    sc_signal<sc_uint<PIXEL_WIDTH>> pix_data;
    sc_signal<sc_uint<ADDRESS_WIDTH>> pix_address;

    int i = 0;
    double total_error = 0;

    /* Connect the DUT */
    image_processor processor("GRAYSCALER");
    processor.enable(enable);
    processor.pix_data(pix_data);
    processor.pix_address(pix_address);

    sc_start(1,SC_NS);

    /* Open VCD file */
    sc_trace_file *wf = sc_create_vcd_trace_file("image_processor");

    /* Dump the desired signals */
    sc_trace(wf, enable, "enable");
    sc_trace(wf, pix_data, "pix_data");
    sc_trace(wf, pix_address, "pix_address");

    /* Initialize all variables */
    for (i=0; i<TEST_ITERATIONS; i++) {
        /* Generate random number */
        random_pixel = rand() % ( 1 << PIXEL_WIDTH );

        pix_data.write(random_pixel);
        sc_start(1,SC_NS);

        /* Start frame */
        enable.write(1);
        sc_start(1,SC_NS);

        /*
        * Test last pixel, since this is PV all pixels are inmediatly
               * available
               */
        total_error += abs((int) pix_data.read() - rgb12_to_gray(random_pixel));
        printf("input pixel: %x\tsystemC output = %x\tpix_data.read() = %x\n",
               random_pixel, (int)pix_data.read(),  rgb12_to_gray(random_pixel));


        enable.write(0);
        sc_start(1,SC_NS);
    }

    cout << "Average Error: " << total_error / TEST_ITERATIONS << endl;


    cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
    sc_close_vcd_trace_file(wf);
    return 0; /* Terminate simulation */
}
