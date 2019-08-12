#include "adc_4_bits.hpp"

void analogicToDigitalConverter_4_bits::processing(){
    double aprox = 0;
    
    if (input.read() > MAX_VOLTAGE){
        digitalValue = VALUES;
#ifdef DEBUG
        cout << "Input value greater than " << MAX_VOLTAGE 
             << "Output clamped to " << VALUES << endl;
#endif
    }
    else if (input.read() < 0){
        digitalValue = 0;
#ifdef DEBUG
        cout << "Negative input value. Output clamped to 0" << endl;
#endif
    }
    else{
        // Using round to minimize the conversion error
        digitalValue = round(input.read() / step);
    }
    digitalValue = digitalValue & 0xF; // To ensure a 4-bits value
    aprox = digitalValue * step;
#ifdef DEBUG
    cout << "Input value: " << input.read() << " @ " << sc_time_stamp() 
         << endl;
    cout << "ADC conversion value: 0x" << hex << digitalValue << " @ " 
         << sc_time_stamp() << endl;
    cout << "Aproximate value: " <<  aprox << endl;
#endif
    output.write(digitalValue);
    error = abs(input.read() - aprox);
    if (error > errorMax){
        errorMax = error;
    }
}

void analogicToDigitalConverter_4_bits::set_attributes() {
    set_timestep(Time_step);
}

void analogicToDigitalConverter_4_bits::initialize() {
}

void analogicToDigitalConverter_4_bits::ac_processing() {
}

