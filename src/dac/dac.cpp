#include "dac.hpp"

/* Reception stage */
void
digital_analog_converter::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        bool command = target->command;
        unsigned short data = target->incoming_buffer;
        wait(sc_time(BUS_DELAY, SC_NS));

        if(command == tlm::TLM_WRITE_COMMAND) {
            pixel = (sc_uint<PIXEL_WIDTH>)(data & 0xFFF);
#ifdef TRANSACTION_PRINT
            cout << "DAC received:\t" << data << " @ " << sc_time_stamp() << endl;
#endif /* TRANSACTION_PRINT*/

            wr_t.notify(sc_time(WRITE_DELAY, SC_NS));
        }
    }
}

void
digital_analog_converter::thread_process()
{
    while(true) {
        wait(wr_t);
        red_channel.write(pixel(3 * CHANNEL_WIDTH - 1, 2 * CHANNEL_WIDTH));
        green_channel.write(pixel(2 * CHANNEL_WIDTH - 1, CHANNEL_WIDTH));
        blue_channel.write(pixel(CHANNEL_WIDTH - 1, 0));
    }
}

