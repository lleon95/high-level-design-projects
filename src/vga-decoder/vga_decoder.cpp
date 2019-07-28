//-----------------------------------------------------
#include "systemc.h"

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
        if (((h_count >= ADDRESSABLE_VIDEO_H_START) &&
                (h_count <= ADDRESSABLE_VIDEO_H_END))   && //Addressable horizontal
                ((v_count >= ADDRESSABLE_VIDEO_V_START) && //Addressable vertical
                 (v_count <= ADDRESSABLE_VIDEO_V_END))) {
            sample_pixel_event.notify(SAMPLING_DELAY, SC_NS);
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
vga_decoder::update_output()
{
    while(true) {
        wait(update_output_event);

        // TLM-2 generic payload transaction, reused across calls to b_transport
        tlm::tlm_generic_payload* trans = new tlm::tlm_generic_payload;
        sc_time delay = sc_time(TRANSACTION_DELAY, SC_NS);
        // Initialize 8 out of the 10 attributes, byte_enable_length being unused
        trans->set_command(tlm::TLM_WRITE_COMMAND);
        trans->set_address(i);  //2 to send it to the processor
        trans->set_data_ptr(reinterpret_cast<unsigned char*>(&pixel));
        trans->set_data_length(2);
        trans->set_streaming_width(2); // = data_length to indicate no streaming
        trans->set_byte_enable_ptr(0); // 0 indicates unused
        trans->set_dmi_allowed(false); // Mandatory initial value
        trans->set_response_status(
            tlm::TLM_INCOMPLETE_RESPONSE); // Mandatory initial value

        cout << "Decoder: Sending pixel " << dec
             << h_count - ADDRESSABLE_VIDEO_H_START << ", " << dec
             << v_count - ADDRESSABLE_VIDEO_V_START << ": 0x" << hex << pixel
             << " to memory address 0x" << hex << i << " @ " << sc_time_stamp()
             << endl;

        initiator_socket->b_transport(*trans, delay);  // Blocking transport call
        i++;
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
vga_decoder::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
{
    tlm::tlm_command cmd = trans.get_command();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    if (byt != 0 || len > PACKAGE_LENGTH || wid < len) {
        SC_REPORT_ERROR("TLM-2",
                        "Target does not support given generic payload transaction");
    }

    if (cmd == tlm::TLM_WRITE_COMMAND) { //Check if this is a writting transaction.
        short* ptr_pixel = reinterpret_cast<short*> (ptr);
        short data = *ptr_pixel;
        cout << "Decoder: Transaction received: 0x" << hex << data << " @ "
             << sc_time_stamp() << endl;
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
    trans.set_response_status(tlm::TLM_OK_RESPONSE);
}

