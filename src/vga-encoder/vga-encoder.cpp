#include <queue>

#include "systemc.h"
#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "vga-encoder.hpp"

SC_MODULE (vga_encoder)
{
    tlm_utils::simple_target_socket<vga_encoder> target_socket;
    tlm_utils::simple_initiator_socket<vga_encoder> initiator_socket;

    /* Pixels Queue */
    std::queue<unsigned short> pixels_queue;

    sc_uint<12> pixel_out;
    sc_out<sc_uint<19> >  pixel_counter;

    sc_out<bool >  h_sync;
    sc_out<bool >  v_sync;

    sc_uint<10>   col; /* 640 cols */
    sc_uint<9>    row; /* 480 rows */
    sc_uint<12> pixel;

    sc_uint<3>  state = 0;
    sc_uint<3>  next_state = 0;

    sc_event wr_t, rd_t, next_state_t, write_pixel;

    SC_HAS_PROCESS(vga_encoder);
    vga_encoder(sc_module_name vga_encoder) {

        SC_THREAD(FSM_Emulator);
        /* Ports */
        SC_THREAD(rd);

        /* Socket */
        target_socket.register_b_transport(this, &vga_encoder::b_transport);
    }

    /* Control stage */
    void FSM_Emulator() {
        while(true) {
            wait(next_state_t);
            state = next_state;
            FSM_next_state();
            FSM_output_logic();
        }
    }

    void FSM_next_state() {
        switch (state) {
        case FSM_VSYNC:
            next_state = FSM_V_BACK_PORCH;
            break;
        case FSM_V_BACK_PORCH:
            next_state = FSM_H_SYNC;
            break;
        case FSM_H_SYNC:
            next_state = FSM_H_BACK_PORCH;
            break;
        case FSM_H_BACK_PORCH:
            next_state = FSM_SEND_PIXELS;
            break;
        case FSM_SEND_PIXELS:
            if (col == COLS - 2) {
                next_state = FSM_H_FRONT_PORCH;
            } else {
                next_state = FSM_SEND_PIXELS;
            }
            break;
        case FSM_H_FRONT_PORCH:
            if (row == ROWS - 1) {
                next_state = FSM_V_FRONT_PORCH;
            } else {
                next_state = FSM_H_SYNC;
            }
            break;
        case FSM_V_FRONT_PORCH:
            next_state = FSM_VSYNC;
            break;
        default:
            next_state = FSM_VSYNC;
            break;
        }
    }

    void FSM_output_logic() {
        switch (state) {
        case FSM_VSYNC:
            v_sync.write(0);
            h_sync.write(1);
            col = 0;
            row = 0;
            next_state_t.notify(DELAY_VSYNC, SC_NS);
            break;
        case FSM_V_BACK_PORCH:
            v_sync.write(1);
            next_state_t.notify(DELAY_V_BACK_PORCH, SC_NS);
            break;
        case FSM_V_FRONT_PORCH:
            v_sync.write(1);
            next_state_t.notify(DELAY_V_FRONT_PORCH, SC_NS);
            break;
        case FSM_H_SYNC:
            h_sync.write(0);
            col = 0;
            next_state_t.notify(DELAY_H_SYNC, SC_NS);
            break;
        case FSM_H_BACK_PORCH:
            h_sync.write(1);
            next_state_t.notify(DELAY_H_BACK_PORCH, SC_NS);
            break;
        case FSM_H_FRONT_PORCH:
            h_sync.write(1);
            row++;
            next_state_t.notify(DELAY_H_FRONT_PORCH, SC_NS);
            break;
        case FSM_SEND_PIXELS:
            h_sync.write(1);
            send_pixel();
            col++;
            next_state_t.notify(DELAY_SEND_PIXELS, SC_PS);
            break;
        default:
            h_sync.write(1);
            v_sync.write(1);
            next_state_t.notify(DELAY_DEFAULT, SC_NS);
            break;
        }
    }

    /* Input ports */
    void reset() {
        col = 0;
        row = 0;
        next_state = FSM_VSYNC;
        next_state_t.notify(READ_DELAY, SC_NS);
    }
    
    /* Output port - Status */
    void read() {
        rd_t.notify(READ_DELAY, SC_NS);
    }
    void rd() {
        while(true) {
            wait(rd_t);
            pixel_counter.write(COLS * row + col);
        }
    }

    /* Datapath */
    void send_pixel() {
        /* Enqueue next pixel */
        pixel_out = pixels_queue.front();
        pixels_queue.pop();
        /* Write to output */
        put_rgb_signal();
    }

    /* TLM implementation */
    virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
    {
        tlm::tlm_command cmd = trans.get_command();
        unsigned char*   ptr = trans.get_data_ptr();
        unsigned int     len = trans.get_data_length();
        unsigned char*   byt = trans.get_byte_enable_ptr();
        unsigned int     wid = trans.get_streaming_width();

        short* ptr_16_bits = reinterpret_cast<short*> (ptr);


        if (byt != 0 || len > PACKAGE_LENGTH || wid < len) {
            SC_REPORT_ERROR("TLM-2",
                "Target does not support given generic payload transaction");
        }

        /* Processor only accepts write operations */
        if ( cmd == tlm::TLM_WRITE_COMMAND ) {
            /* Copy pixels to internal buffer */
            unsigned short pixel_in = *(ptr_16_bits) & 0xFFF; 
            
            /* Write pixels into queue */
            pixels_queue.push(pixel_in);

            trans.set_response_status( tlm::TLM_OK_RESPONSE );
        }
    }

    void put_rgb_signal()
    {
        tlm::tlm_generic_payload trans;

        /* Pointer to be passed to the target, target is resposible for freeing it */
        int * return_pixel = new int;
        sc_time delay = sc_time(INTERRUPT_DELAY, SC_NS);

        *(return_pixel) = (int)pixel_out;
        trans.set_address( DESTINATION_ADDRESS );
        trans.set_command( tlm::TLM_WRITE_COMMAND );
        trans.set_data_ptr( reinterpret_cast<unsigned char*>(return_pixel) );
        trans.set_data_length( 2 );
        trans.set_streaming_width( 2 ); /* = data_length to indicate no streaming */
        trans.set_byte_enable_ptr( 0 ); /* 0 indicates unused */
        trans.set_dmi_allowed( false ); /* Mandatory initial value */
        trans.set_response_status(
            tlm::TLM_INCOMPLETE_RESPONSE ); /* Mandatory initial value */

        initiator_socket->b_transport( trans, delay );  // Blocking transport call
    }

};
