#include <systemc-ams.h>
#include <systemc.h>
#include <math.h>
#include "vga_decoder.hpp"  //To use PIXEL_DELAY

/*
**
****************************************************
**          Module description
****************************************************
**  SIZE bits unipolar analogic to digital converter
**  with a MAX_VOLTAGE v maximum input value.
**  Sampling period = SAMPLING_PERIOD nano seconds
**
*/

#define SIZE 4
#define VALUES ((1 << SIZE) - 1) // Unipolar ADC because of it's resolution.
#define MAX_VOLTAGE 5.00
#define SAMPLING_PERIOD PIXEL_DELAY // In nano seconds

SCA_TDF_MODULE(analogicToDigitalConverter_4_bits)
{
    sca_tdf::sca_de::sca_in<double> input; // TDF port
    sca_tdf::sca_de::sca_out<short> output; // TDF port
    sca_core::sca_time Time_step;
    short digitalValue; // This models the register
    double step;        // Holds the ADC LSB weight
    double error;       // Holds the last-conversion error
    double errorMax;    // Holds the maximum conversion error

    void processing();
    void set_attributes();
    void initialize();
    void ac_processing();

    SCA_CTOR(analogicToDigitalConverter_4_bits) : input("Input"), output("Output"){
        Time_step = sca_core::sca_time(SAMPLING_PERIOD, sc_core::SC_NS);
        digitalValue = 0;
        step = MAX_VOLTAGE / VALUES;
        error = 0;
        errorMax = 0;
    }
};

