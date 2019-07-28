//-----------------------------------------------------
#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#define RESOLUTION 640*480
#define PIXEL_SIZE 12
#define PIXEL_POS 0
#define H_SYNC_POS 12
#define V_SYNC_POS 13
#define GET_PIXEL(DATA) (DATA & 0xFFF)
#define GET_H_SYNC(DATA) ((DATA & 1 << H_SYNC_POS) >> H_SYNC_POS)
#define GET_V_SYNC(DATA) ((DATA & 1 << V_SYNC_POS) >> V_SYNC_POS)
#define PIXEL_DELAY  39.722  //This is in nano secs
#define PIXELS_IN_ROW 800
#define ROWS_IN_FRAME 525
#define ROW_DELAY PIXEL_DELAY * PIXELS_IN_ROW // This is in nano secs
#define UPDATE_OUTPUT_DELAY 10 //nanoseconds
#define SAMPLING_DELAY 10 //nanoseconds
#define ADDRESSABLE_VIDEO_H_START 145
#define ADDRESSABLE_VIDEO_H_END 784
#define ADDRESSABLE_VIDEO_V_START 36
#define ADDRESSABLE_VIDEO_V_END 515
#define NOTIFY_FRAME_START_INTERRUPT_DELAY 10 //nanoseconds
#define PACKAGE_LENGTH 2
#define TRANSACTION_DELAY 10 //Nano seconds

// Needed for the simple_target_socket
#define SC_INCLUDE_DYNAMIC_PROCESSES

struct vga_decoder : sc_module {
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_initiator_socket<vga_decoder> initiator_socket;
    tlm_utils::simple_target_socket<vga_decoder> target_socket;

    //-----------Internal variables-------------------
    int i;                //Internal counter to know the address we write.
    short pixel_in;
    short pixel;
    bool previous_h_sync;
    bool current_h_sync;
    bool previous_v_sync;
    bool current_v_sync;

    int h_count, v_count;        // Count for the column and row.

    sc_event count_column_event; //Event to notify the start of a new column
    sc_event count_row_event;    //Event to notify the start of a new row
    sc_event update_output_event;
    sc_event decode_pixel_event;
    sc_event sample_pixel_event;

    void start_column_count();
    void start_row_count();
    void decode_pixel();
    void column_count();
    void row_count();
    void update_output();
    void sample_pixel();

    // Function to receive transactions
    virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);

    SC_CTOR(vga_decoder) : initiator_socket("socket")
    {

        h_count  = 0;
        v_count  = 0;
        pixel_in = 0;
        pixel    = 0;
        i        = 0;

        previous_h_sync = 1;
        current_h_sync  = 1;
        previous_v_sync = 1;
        current_v_sync  = 1;

        SC_THREAD(decode_pixel);
        SC_THREAD(column_count);
        SC_THREAD(row_count);
        SC_THREAD(sample_pixel);
        SC_THREAD(update_output);

        target_socket.register_b_transport(this, &vga_decoder::b_transport);
    } //End of Constructor
}; // End of module vga_decoder

