#include "adc.hpp"

short analogic_digital_converter::getDigitalValue(){
    short temp_red   = adc_channel_red->digitalValue;
    short temp_green = adc_channel_green->digitalValue;
    short temp_blue  = adc_channel_blue->digitalValue;
    digitalValue =      SHIFT_LEFT_VALUE(temp_red, RED_PIXEL_START) |
                        SHIFT_LEFT_VALUE(temp_green, GREEN_PIXEL_START) |
                        SHIFT_LEFT_VALUE(temp_blue, BLUE_PIXEL_START);
    return digitalValue;
}

void  analogic_digital_converter::thread_process()
{
    while(true) {
        short data = getDigitalValue();
#ifdef TRANSACTION_PRINT
        cout << "ADC writing:\t" << data << " @ " << sc_time_stamp() << endl;
#endif /* TRANSACTION_PRINT */
        initiator->write(DECODER_ADDRESS, (int)data, tlm::TLM_WRITE_COMMAND);
        wait(sc_time(PIXEL_DELAY, SC_NS)); // PIXEL_DELAY nano seconds elapsed
    }
}

void analogic_digital_converter::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        unsigned short data = target->incoming_buffer;
#ifdef TRANSACTION_PRINT
        //We shouldn't receive any transactions, but if we do ...
        cerr << "ADC: ERROR - Transaction received: 0x" << hex << data
             << " @ " << sc_time_stamp() << endl;
#endif /* TRANSACTION_PRINT */
    }
}

