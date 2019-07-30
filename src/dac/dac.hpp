#ifndef DAC_HPP
#define DAC_HPP

#include "systemc.h"
#include "tlm.h"
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
    sc_out<sc_uint<CHANNEL_WIDTH> > red_channel;
    sc_out<sc_uint<CHANNEL_WIDTH> > green_channel;
    sc_out<sc_uint<CHANNEL_WIDTH> > blue_channel;
    sc_uint<PIXEL_WIDTH> pixel;

    /* Events */
    sc_event wr_t;

    SC_HAS_PROCESS(digital_analog_converter);
    digital_analog_converter(const sc_module_name & name) : Node(name) {
    }

    /* Control units */
    void thread_process();
    void reading_process();

    /* Data path */
    void put_rgb_signal();

};
#endif

