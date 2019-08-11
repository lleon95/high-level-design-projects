//-----------------------------------------------------
#include "systemc.h"
#include "tlm.h"
#include "node.hpp"

#define PIXEL_SIZE 12
#define MAX_PIXEL_VALUE_PLUS_ONE (1 << PIXEL_SIZE)

#define PIXEL_POS 0
#define H_SYNC_POS 12
#define V_SYNC_POS 13

#define ROWS_IN_FRAME 525   // Rows in a screen, not the visible ones
#define FRAMES 1             // Frames to simulate
#define SIMULATION_TIME (ROW_DELAY * ROWS_IN_FRAME * FRAMES) // In nano seconds
#define H_SYNC_SYNCH_PULSE_LENGHT 96 //In pixels
#define V_SYNC_SYNCH_PULSE_LENGHT 2  //In rows

#define PIXEL_DELAY  39.722  //This is in nano secs
#define PIXELS_IN_ROW 800
#define ROW_DELAY (PIXEL_DELAY * PIXELS_IN_ROW) // This is in nano secs

#define ADDRESSABLE_VIDEO_H_START 145
#define ADDRESSABLE_VIDEO_H_END 784
#define ADDRESSABLE_VIDEO_V_START 36
#define ADDRESSABLE_VIDEO_V_END 515

#define PACKAGE_LENGTH 2
#define PACKAGE_LENGH_IN_BITS (PACKAGE_LENGTH * 8)
#define TRANSACTION_DELAY 10 //Nano seconds
#define CHANNEL_WIDTH 4
#define PIXEL_WIDTH 12

#ifdef DEBUG
#define DEBUG_PIXELS 5
#define DEBUG_H_SYNC_SYNCH_PULSE_LENGTH 2
#endif /* DEBUG */


struct analogic_digital_converter : Node {
    /* I/O */
    sc_in<sc_uint<CHANNEL_WIDTH> > red_channel;
    sc_in<sc_uint<CHANNEL_WIDTH> > green_channel;
    sc_in<sc_uint<CHANNEL_WIDTH> > blue_channel;
  
    void thread_process();
    void reading_process();
    analogic_digital_converter(const sc_module_name & name) : Node(name)
    {
    } //End of constructor
}; // End of adc module

