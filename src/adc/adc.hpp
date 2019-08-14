#ifndef _ADC_
#define _ADC_

#include "adc_4_bits.hpp"
#include "node.hpp"
#include "router.hpp"

/*
**
****************************************************
**          Module description
****************************************************
**  3 channels analogic to digital converter
**  See adc_4_bits.hpp for details about a single
**  channel implementation.
**
***************************************************
*/

#define SHIFT_LEFT_VALUE(value, shift) (value << shift)
#define RED_PIXEL_START 0
#define GREEN_PIXEL_START 4
#define BLUE_PIXEL_START 8

/* 
**
***************************************************
**                      PACKAGE
***************************************************
---------------------------------------------------------
 15 14 |   13   |   12   |  11 - 8  |  7 - 4  |  3 - 0  |  
---------------------------------------------------------
UNUSED | V SYNC | H SYNC |   BLUE   |  GREEN  |   RED   |
---------------------------------------------------------
***************************************************
*/


struct analogicToDigitalConverter : Node{
    sc_core::sc_in<double> input_red;
    sc_core::sc_in<double> input_green;
    sc_core::sc_in<double> input_blue;
    sc_core::sc_out<short> output_red;
    sc_core::sc_out<short> output_green;
    sc_core::sc_out<short> output_blue;
    analogicToDigitalConverter_4_bits *adc_channel_red;
    analogicToDigitalConverter_4_bits *adc_channel_green;
    analogicToDigitalConverter_4_bits *adc_channel_blue;

    short digitalValue; // This models the register.

    short getDigitalValue();
    void thread_process();
    void reading_process();

    analogicToDigitalConverter(const sc_module_name & name) : 
                                            Node(name), 
                                            input_red("Input_RED"),
                                            input_green("Input_GREEN"),
                                            input_blue("Input_BLUE"),
                                            output_red("Output_RED"),
                                            output_green("Output_GREEN"),
                                            output_blue("Output_BLUE"){
        digitalValue = 0;
        adc_channel_red = new analogicToDigitalConverter_4_bits("channel_RED");
        adc_channel_green = new 
            analogicToDigitalConverter_4_bits("channel_GREEN");
        adc_channel_blue = new 
            analogicToDigitalConverter_4_bits("channel_BLUE");
        adc_channel_red->input(input_red);
        adc_channel_red->output(output_red);
        adc_channel_green->input(input_green);
        adc_channel_green->output(output_green);
        adc_channel_blue->input(input_blue);
        adc_channel_blue->output(output_blue);
    }
};
#endif /* _ADC_ */
