#include "dac.hpp"

/* Reception stage */
void
dac::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
{
    tlm::tlm_command cmd = trans.get_command();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    short* ptr_16_bits = reinterpret_cast<short*> (ptr);


    if (byt != 0 || len > PACKAGE_LENGTH || wid < len) {
        SC_REPORT_ERROR("TLM-2",
                        "Target does not support given generic payload transaction");
    }

    /* Processor only accepts write operations */
    if ( cmd == tlm::TLM_WRITE_COMMAND ) {
        /* Copy pixels to internal buffer */
        pixel = (sc_uint<PIXEL_WIDTH>)(*(ptr_16_bits) & 0xFFF);

        /* Write pixels into queue */
        wr_t.notify(sc_time(WRITE_DELAY, SC_NS));
 
        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    }
}

void
dac::put_rgb_signal()
{
    while(true) {
        wait(wr_t);
        red_channel.write(pixel(3 * CHANNEL_WIDTH - 1, 2 * CHANNEL_WIDTH));
        green_channel.write(pixel(2 * CHANNEL_WIDTH - 1, CHANNEL_WIDTH));
        blue_channel.write(pixel(CHANNEL_WIDTH - 1, 0));
    }
}

