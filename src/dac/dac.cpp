#include "dac.hpp"

/* Reception stage */
void
dac::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        bool command = target->command;
        unsigned short data = target->incoming_buffer;
        wait(sc_time(BUS_DELAY, SC_NS));

        if(command == tlm::TLM_WRITE_COMMAND) {
            pixel = (sc_uint<PIXEL_WIDTH>)(data & 0xFFF);

            wr_t.notify(sc_time(WRITE_DELAY, SC_NS));
        }
    }
}

void
dac::thread_process()
{
    while(true) {
        wait(wr_t);
        red_channel.write(pixel(3 * CHANNEL_WIDTH - 1, 2 * CHANNEL_WIDTH));
        green_channel.write(pixel(2 * CHANNEL_WIDTH - 1, CHANNEL_WIDTH));
        blue_channel.write(pixel(CHANNEL_WIDTH - 1, 0));
    }
}
