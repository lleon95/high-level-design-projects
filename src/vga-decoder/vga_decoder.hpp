//-----------------------------------------------------
#include "systemc.h"
#include "tlm.h"
#include "node.hpp"

/* EXPECTED PACKAGE
-------------------------------------------------------
 15 14 |   13   |   12   |           11 - 0           |
-------------------------------------------------------
UNUSED | V SYNC | H SYNC |           PIXEL            |
-------------------------------------------------------
*/

#define RESOLUTION (640*480)
#define PIXEL_WIDTH 12
#define PIXEL_DELAY  39.722  //This is in nano secs
#define ROWS_IN_SCREEN 525   // Rows in a screen, not the visible ones
#define PIXELS_IN_ROW 800

#define GET_PIXEL(DATA) (DATA & 0xFFF)

#define PACKAGE_SIZE_IN_BYTES 2
#define PACKAGE_LENGTH_IN_BITS (PACKAGE_SIZE_IN_BYTES * 8)

#ifdef DEBUG
#define DEBUG_PIXELS 5
#define DEBUG_ADDRESSABLE_VIDEO_H_START 2
#define DEBUG_MAX_PIXELS_TO_SEND (DEBUG_PIXELS - DEBUG_ADDRESSABLE_VIDEO_H_START + 1)

#define DEBUG_PIXEL_IN_ROW 10
#define DEBUG_ROWS_IN_FRAME 10
#define DEBUG_ROW_DELAY (DEBUG_PIXEL_IN_ROW * PIXEL_DELAY)
#endif /* DEBUG */

struct vga_decoder : Node {
    /* I/O signals */
    sc_in<bool> h_sync;
    sc_in<bool> v_sync;

    //-----------Internal variables-------------------
    short pixel_in;
    short pixel;
    bool previous_h_sync;
    bool previous_v_sync;

    int h_count, v_count;        // Count for the column and row.
#ifdef DEBUG
    int pixels_transmitted;  //Used to stop the simulation once we sent the
    //DEBUG mode pixels.
#endif
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
    void sample_pixel();

    void thread_process();
    void reading_process();

    SC_HAS_PROCESS(vga_decoder);
    vga_decoder(const sc_module_name & name) : Node(name)
    {

        h_count  = 0;
        v_count  = 0;
        pixel_in = 0;
        pixel    = 0;

#ifdef DEBUG
        pixels_transmitted = 0;
#endif

        previous_h_sync = 1;
        previous_v_sync = 1;

        SC_THREAD(decode_pixel);
        SC_THREAD(column_count);
        SC_THREAD(row_count);
        SC_THREAD(sample_pixel);

    } //End of Constructor
}; // End of module vga_decoder

