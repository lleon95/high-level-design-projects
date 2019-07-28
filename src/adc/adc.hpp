//-----------------------------------------------------
#include "systemc.h"
#include "tlm.h"
#include "node.hpp"

#define RESOLUTION 640*480
#define PIXEL_SIZE 12
#define PIXEL_POS 0
#define H_SYNC_POS 12
#define V_SYNC_POS 13

#define ROWS_IN_FRAME 525   // Rows in a screen, not the visible ones
#define FRAMES 1             // Frames to simulate
#define SIMULATION_TIME 5 * PIXEL_DELAY // ROW_DELAY * ROWS_IN_FRAME * FRAMES // In nano seconds
#define H_SYNC_SYNCH_PULSE_LENGHT 96 //In pixels
#define V_SYNC_SYNCH_PULSE_LENGHT 2  //In rows

#define PIXEL_DELAY  39.722  //This is in nano secs
#define PIXELS_IN_ROW 800

#define ROW_DELAY PIXEL_DELAY * PIXELS_IN_ROW // This is in nano secs
#define ADDRESSABLE_VIDEO_H_START 145
#define ADDRESSABLE_VIDEO_H_END 784
#define ADDRESSABLE_VIDEO_V_START 36
#define ADDRESSABLE_VIDEO_V_END 515
#define PACKAGE_LENGTH 2
#define TRANSACTION_DELAY 10 //Nano seconds

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

// ADC module generating generic payload transactions

struct adc : Node
{   
    void thread_process();
    void reading_process();
    
    adc(const sc_module_name & name) : Node(name){
    } //End of constructor
}; // End of adc module

