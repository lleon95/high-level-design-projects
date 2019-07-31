//-----------------------------------------------------

#include "vga_decoder.hpp"

#define ROWS_IN_FRAME 525
#define ROW_DELAY (PIXEL_DELAY * PIXELS_IN_ROW) // This is in nano secs
#define UPDATE_OUTPUT_DELAY 10 //nanoseconds
#define SAMPLING_DELAY 10 //nanoseconds
#define ADDRESSABLE_VIDEO_H_START 145
#define ADDRESSABLE_VIDEO_H_END 784
#define ADDRESSABLE_VIDEO_V_START 36
#define ADDRESSABLE_VIDEO_V_END 515

#define PACKAGE_SIZE_IN_BYTES 2
#define PACKAGE_LENGTH_IN_BITS (PACKAGE_SIZE_IN_BYTES * 8)

//------------Code Starts Here-------------------------
void
vga_decoder::start_column_count()
{
    h_count = 0;
    count_column_event.notify();
}

void
vga_decoder::start_row_count()
{
    v_count = 0;
    count_row_event.notify();
}

void
vga_decoder::decode_pixel()
{
    while(true) {
        wait(decode_pixel_event);
        if (((h_count >= ADDRESSABLE_VIDEO_H_START) &&
                (h_count <= ADDRESSABLE_VIDEO_H_END))   && //Addressable horizontal
                ((v_count >= ADDRESSABLE_VIDEO_V_START) && //Addressable vertical
                 (v_count <= ADDRESSABLE_VIDEO_V_END))) {
            if (pixels_transmitted < DEBUG_MAX_PIXELS_TO_SEND) {
                sample_pixel_event.notify(SAMPLING_DELAY, SC_NS);
            }
        }
    }
}

void
vga_decoder::column_count()
{
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

void
vga_decoder::row_count()
{
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

void
vga_decoder::sample_pixel()
{
    while(true) {
        wait(sample_pixel_event);
        pixel = pixel_in;
        update_output_event.notify(UPDATE_OUTPUT_DELAY, SC_NS);
    }
}

void
vga_decoder::thread_process()
{
    while(true) {
        wait(update_output_event);
#ifdef TRANSACTION_PRINT
        cout << "Decoder sending:\t" << pixel << " @ " << sc_time_stamp() << endl;
#endif /* TRANSACTION_PRINT */
        initiator->write(CPU_ADDRESS, (int)pixel, tlm::TLM_WRITE_COMMAND);
        wait(sc_time(BUS_DELAY, SC_NS));
    }
}

void
vga_decoder::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        bool cmd = target->command;
        unsigned short data = target->incoming_buffer;

        if (cmd == tlm::TLM_WRITE_COMMAND) {
#ifdef TRANSACTION_PRINT
            cout << "Decoder received:\t" << data << " @ " << sc_time_stamp() << endl;
#endif /* TRANSACTION_PRINT */
            pixel_in = GET_PIXEL(data);
            if ((h_sync == 0) && (previous_h_sync == 1)) { //Falling egde in h sync
                start_column_count();
            }

            if ((v_sync == 0) && (previous_v_sync == 1)) { //Falling egde in v sync
                start_row_count();
            }

            previous_h_sync = h_sync;
            previous_v_sync = v_sync;
        }
    }
}
