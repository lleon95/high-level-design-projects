#ifndef DAC_HPP
#define DAC_HPP

#include "systemc.h"
#include "tlm.h"

#include "analog-4bit-dac.hpp"
#include "node.hpp"

/* Signal delays */
#define WRITE_DELAY 5 /* 10ns */

/* Signals width */
#define PIXEL_WIDTH 12
#define CHANNEL_WIDTH 4
#define PACKAGE_LENGTH 2 /* 16 bits package length */

struct digital_analog_converter : Node
{
    /* I/O */
    sc_signal<sc_uint<PIXEL_WIDTH> > pixel; /* Acts as the register*/
    sca_eln::sca_terminal r_channel;
    sca_eln::sca_terminal g_channel;
    sca_eln::sca_terminal b_channel;
    sc_signal<sc_uint<4> > r_in;
    sc_signal<sc_uint<4> > g_in;
    sc_signal<sc_uint<4> > b_in;

    /* Analog part */
    fourbit_dac * dac4bit = new fourbit_dac("dac4bit");
    digital_dac_interface * digital_iface_r = new digital_dac_interface("digital_iface_r");
    digital_dac_interface * digital_iface_g = new digital_dac_interface("digital_iface_g");
    digital_dac_interface * digital_iface_b = new digital_dac_interface("digital_iface_b");
    /* Events */
    sc_event wr_t;

    SC_HAS_PROCESS(digital_analog_converter);
    digital_analog_converter(const sc_module_name & name) : Node(name) {
      /* Create one 4-bit dac per channel */
      digital_iface_r->sigOutput(r_channel);
      digital_iface_g->sigOutput(g_channel);
      digital_iface_b->sigOutput(b_channel);
      /* Split the pixel in its components */
      digital_iface_r->in(r_in);
      digital_iface_g->in(g_in);
      digital_iface_b->in(b_in);
    }

    /* Control units */
    void thread_process();
    void reading_process();

    void update_process();

    /* Data path */
    void put_rgb_signal();

};
#endif

