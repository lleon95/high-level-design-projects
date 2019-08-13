#include "adc_4_bits.hpp"

#define T (5 * PIXEL_DELAY)  // nanoseconds. Test-signal period
#define F (1 / T) // Test-signal frequency. In GHz

int sc_main(int argc, char* argv[]){
    
    sc_core::sc_signal<double> input;
    sc_core::sc_signal<short> output;
    analogicToDigitalConverter_4_bits *converter =
        new analogicToDigitalConverter_4_bits("Dummy");
    
    //Connect the DUT
    converter->input(input);
    converter->output(output);
    
    /* Log file */
    sca_util::sca_trace_file *tdf =
    sca_util::sca_create_vcd_trace_file("adc_4_bits.vcd");
    sca_trace(tdf, input, "Input");
    sca_trace(tdf, output, "Output");
    for(int t=0; t < (5 * T) ; t += (PIXEL_DELAY / 10)){
        input.write((MAX_VOLTAGE / 2.00) * (sin(2 * M_PI * F * t) + 1));
        sc_start(PIXEL_DELAY / 10, SC_NS);
    }
    cout << "Maximum error: " << converter->errorMax << endl;
    cout << "LSB: " << converter->step << endl;
    sca_util::sca_close_vcd_trace_file(tdf);
    return 0;
}

