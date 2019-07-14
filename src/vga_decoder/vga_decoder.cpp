//-----------------------------------------------------
#include "systemc.h"

#define PIXEL_SIZE 12
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
#define INTERRUPT_TIME 1 //nanoseconds
#define NOTIFY_FRAME_START_INTERRUPT_DELAY 10 //nanoseconds

SC_MODULE (vga_decoder)
{

    //-----------Inputs/Outputs-----------------------

    sc_in<bool> hsync;      // H SYNC signal
    sc_in<bool> vsync;      // V SYNC signal
    sc_in<sc_uint<PIXEL_SIZE> > pixel_in;

    sc_out<sc_uint<PIXEL_SIZE> > pixel_out;
    sc_out<bool> frame_start; // Interrupt to represent a frame start.

    //-----------Internal variables-------------------

    uint pixel;
    bool enable_interrupt;       // Flag to show if the interrupt can be triggered
    int h_count, v_count;        // Count for the column and row.
    sc_event count_column_event; //Event to notify the start of a new column
    sc_event count_row_event;    //Event to notify the start of a new row
    sc_event update_output_event;
    sc_event decode_pixel_event;
    sc_event sample_pixel_event;
    sc_event clear_interrupt_event;
    sc_event frame_start_interrupt_control_event;

    SC_HAS_PROCESS(vga_decoder);

    vga_decoder(sc_module_name vga_decoder) {

        h_count = 0;
        v_count = 0;
        pixel   = 0;
        enable_interrupt = 0;

        SC_METHOD (start_column_count);
        sensitive << hsync.neg();

        SC_METHOD (start_row_count);
        sensitive << vsync.neg();

        SC_THREAD(decode_pixel);
        SC_THREAD(column_count);
        SC_THREAD(row_count);
        SC_THREAD(sample_pixel);
        SC_THREAD(update_output);
        SC_THREAD(frame_start_interrupt_control);
        SC_THREAD(clear_interrupt);
    }

    //------------Code Starts Here-------------------------

    void start_column_count() {
        h_count = 0;
        count_column_event.notify();
    }

    void start_row_count() {
        v_count = 0;
        count_row_event.notify();
        enable_interrupt = 1;
    }

    void decode_pixel() {
        while(true) {
            wait(decode_pixel_event);
            if (((h_count >= ADDRESSABLE_VIDEO_H_START)       &&
                    (h_count <= ADDRESSABLE_VIDEO_H_END))     && //Addressable horizontal
                    ((v_count >= ADDRESSABLE_VIDEO_V_START)   && //Addressable vertical
                     (v_count <= ADDRESSABLE_VIDEO_H_START))) {
                sample_pixel_event.notify(SAMPLING_DELAY, SC_NS);
            }
        }
    }

    void column_count() {
        while(true) {
            wait(count_column_event);
            if (h_count < PIXELS_IN_ROW) {
                h_count += 1;
                decode_pixel_event.notify();
                if (h_count != PIXELS_IN_ROW) {
                    count_column_event.notify(PIXEL_DELAY, SC_NS);
                }
            }
        }
    }

    void row_count() {
        while(true) {
            wait(count_row_event);
            if (v_count < ROWS_IN_FRAME) {
                v_count += 1;
                if (v_count != ROWS_IN_FRAME) {
                    count_row_event.notify(ROW_DELAY, SC_NS);
                }
            }
        }
    }

    void update_output() {
        while(true) {
            wait(update_output_event);
            pixel_out.write(pixel);
            frame_start_interrupt_control_event.notify(NOTIFY_FRAME_START_INTERRUPT_DELAY,
                    SC_NS);
        }
    }

    void frame_start_interrupt_control() {
        while(true) {
            wait(frame_start_interrupt_control_event);
            if(enable_interrupt) {
                if (h_count == ADDRESSABLE_VIDEO_H_START &&
                        v_count == ADDRESSABLE_VIDEO_V_START) {
                    frame_start.write(1);
                    clear_interrupt_event.notify(INTERRUPT_TIME, SC_NS);
                }
            } else {
                frame_start.write(0);
            }
        }
    }

    void clear_interrupt() {
        while(true) {
            wait(clear_interrupt_event);
            enable_interrupt = 0;
        }
    }

    void sample_pixel() {
        while(true) {
            wait(sample_pixel_event);
            pixel = pixel_in.read();
            update_output_event.notify(UPDATE_OUTPUT_DELAY, SC_NS);
        }
    }

}; // End of Module vga_decoder

