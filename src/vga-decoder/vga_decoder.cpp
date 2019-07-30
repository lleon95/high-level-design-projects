//-----------------------------------------------------

#include "vga_decoder.hpp"

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
#ifdef DEBUG
        if (h_count >= DEBUG_ADDRESSABLE_VIDEO_H_START) {
#else
        if (((h_count >= ADDRESSABLE_VIDEO_H_START) &&
                (h_count <= ADDRESSABLE_VIDEO_H_END))   && //Addressable horizontal
                ((v_count >= ADDRESSABLE_VIDEO_V_START) && //Addressable vertical
                 (v_count <= ADDRESSABLE_VIDEO_V_END))) {
#endif
#ifdef DEBUG
            if (pixels_transmitted < DEBUG_MAX_PIXELS_TO_SEND) {
#endif
                sample_pixel_event.notify(SAMPLING_DELAY, SC_NS);
#ifdef DEBUG
            }
#endif
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
            cout << "Decoder: Sending pixel : 0x" << hex << pixel << endl;
            initiator->write(CPU_ADDRESS - 1, (int)pixel, tlm::TLM_WRITE_COMMAND);
            wait(sc_time(BUS_DELAY, SC_NS));
#ifdef DEBUG
        pixels_transmitted++;
#endif
    }
}

void
vga_decoder::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        bool cmd = target->command;
        unsigned short data = target->incoming_buffer;
        wait(sc_time(BUS_DELAY, SC_NS));

        if (cmd == tlm::TLM_WRITE_COMMAND) {
            cout << "Decoder: Transaction received: 0x" << hex << data << endl;
            pixel_in = GET_PIXEL(data);
            current_h_sync = GET_H_SYNC(data);
            current_v_sync = GET_V_SYNC(data);
            if ((current_h_sync == 0) && (previous_h_sync == 1)) { //Falling egde in h sync
                start_column_count();
            }

            if ((current_v_sync == 0) && (previous_v_sync == 1)) { //Falling egde in v sync
                start_row_count();
            }

            previous_h_sync = current_h_sync;
            previous_v_sync = current_v_sync;
        }
    }
}
