//-----------------------------------------------------

#include "adc.hpp"

//------------Code Starts Here-------------------------
void
analogic_digital_converter::thread_process()
{
    while(true) {
        sc_uint<PIXEL_WIDTH> data = 0;

        data.range(11, 8) = red_channel;
        data.range(7, 4) = green_channel;
        data.range(3, 0) = blue_channel;

        cout << "ADC writing:\t" << data << " @ " << sc_time_stamp() << endl;
        initiator->write(DECODER_ADDRESS, (int)data, tlm::TLM_WRITE_COMMAND);
        wait(sc_time(PIXEL_DELAY, SC_NS));   // PIXEL_DELAY nano seconds elapsed
    }
}

void
analogic_digital_converter::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        unsigned short data = target->incoming_buffer;
        //We shouldn't receive any transactions, but if we do ...
        cerr << "ADC: ERROR - Transaction received: 0x" << hex << data
             << " @ " << sc_time_stamp() << endl;
    }
}
