#ifndef DAC_HPP
#define DAC_HPP

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

/* Signal delays */
#define WRITE_DELAY 5 /* 10ns */

/* Signals width */
#define PIXEL_WIDTH 12
#define CHANNEL_WIDTH 4

struct dac : sc_module
{
    tlm_utils::simple_target_socket<vga_encoder> target_socket;
    tlm_utils::simple_initiator_socket<vga_encoder> initiator_socket;

    /* I/O */
    sc_out<sc_uint<CHANNEL_WIDTH> > red_channel;
    sc_out<sc_uint<CHANNEL_WIDTH> > green_channel;
    sc_out<sc_uint<CHANNEL_WIDTH> > blue_channel;
    sc_uint<PIXEL_WIDTH> pixel;

    /* Events */
    sc_event wr_t;

    SC_HAS_PROCESS(dac);
    dac(sc_module_name dac) {

        SC_THREAD(put_rgb_signal);

        target_socket.register_b_transport(this, &vga_encoder::b_transport);
    }

    /* Datapath */
    void send_pixel();

    /* TLM implementation */
    virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);

    /* Data path */
    void put_rgb_signal();

};
#endif

