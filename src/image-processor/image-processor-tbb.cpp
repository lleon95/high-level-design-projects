
#include <math.h>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>

#include "image-processor.cpp"

#define TEST_ITERATIONS 20

/* Get the exact expected value */
double
rgb12_to_gray(int pixel)
{
    int r = (pixel >> 8) & 0xF;
    int g = (pixel >> 4) & 0xF;
    int b = pixel & 0xF;

    return r * 0.3 + g * 0.59 + b * 0.11;
}

int
sc_main (int argc, char* argv[])
{
    int random_pixel = 0;
    srand (time(NULL));

    sc_signal<bool>   enable;
    sc_signal<sc_uint<PIXEL_IN_WIDTH>> pix_in;
    sc_signal<sc_uint<PIXEL_OUT_WIDTH>> pix_out;

    int i = 0;
    double total_error = 0;
    /* Connect the DUT */
    image_processor processor("GRAYSCALER");
    processor.enable(enable);
    processor.pix_out(pix_out);
    processor.pix_in(pix_in);

    sc_start(1,SC_NS);

    /* Open VCD file */
    sc_trace_file *wf = sc_create_vcd_trace_file("grayscaler");
    /* Dump the desired signals */
    sc_trace(wf, enable, "enable");
    sc_trace(wf, pix_in, "pix_in");
    sc_trace(wf, pix_in, "pix_in");
    sc_trace(wf, pix_out, "pix_out");

    /* Initialize all variables */
    cout << "@" << sc_time_stamp() <<" Asserting Enable\n" << endl;
    enable = 1;  /* Assert enable */
    for (i=0; i<TEST_ITERATIONS; i++) {
        /* Generate random number */
        random_pixel = rand() % ( 1 << PIXEL_IN_WIDTH );
        pix_in.write(random_pixel);
        sc_start(1,SC_NS);
        total_error += abs(pix_out.read() - rgb12_to_gray(random_pixel));
        cout << "input pixel: " << random_pixel << "\tsystemC output = " <<
             pix_out.read() << "\texpected output = " << rgb12_to_gray(random_pixel) << endl;
    }

    cout << "Average Error: " << total_error / TEST_ITERATIONS << endl;


    cout << "@" << sc_time_stamp() <<" De-Asserting Enable\n" << endl;
    enable = 0; /* De-assert enable */


    cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
    sc_close_vcd_trace_file(wf);
    return 0; /* Terminate simulation */
}
