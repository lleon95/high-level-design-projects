#include "systemc.h"

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "vga-encoder.cpp"

#define RUNTIME 17000000000 /* 17ms */
#define RUNTIME_STEP 10000 /* 10ns */
#define CHANNEL_WIDTH 4

#define COLOR_COL_WIDTH 20

sc_uint<12> pixel_compute ();

struct test_logger : sc_module {
    tlm_utils::simple_target_socket<test_logger> target_socket;
    tlm_utils::simple_initiator_socket<test_logger> initiator_socket;

    int return_pixel;

    void
    pixel_send(int data)
    {
        tlm::tlm_generic_payload trans;
        /* Pointer to be passed to the target, target is resposible for freeing it */
        int* return_pixel = new int;
        sc_time delay = sc_time(INTERRUPT_DELAY, SC_NS);

        *return_pixel = data;

        trans.set_command( tlm::TLM_WRITE_COMMAND );
        trans.set_data_ptr( reinterpret_cast<unsigned char*>(&data) );
        trans.set_data_length( 2 );
        trans.set_streaming_width( 2 ); /* = data_length to indicate no streaming */
        trans.set_byte_enable_ptr( 0 ); /* 0 indicates unused */
        trans.set_dmi_allowed( false ); /* Mandatory initial value */
        trans.set_response_status(
            tlm::TLM_INCOMPLETE_RESPONSE ); /* Mandatory initial value */

        initiator_socket->b_transport( trans, delay );  // Blocking transport call
    }

    /* Function to receive transfers  */
    virtual void
    b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
    {
        tlm::tlm_command cmd = trans.get_command();
        unsigned char*   ptr = trans.get_data_ptr();
        unsigned int     len = trans.get_data_length();
        unsigned char*   byt = trans.get_byte_enable_ptr();
        unsigned int     wid = trans.get_streaming_width();

        short* ptr_16_bits = reinterpret_cast<short*> (ptr);
        sc_uint<PIXEL_WIDTH> current_pixel = 0;

        if (byt != 0 || len > PACKAGE_LENGTH || wid < len) {
            SC_REPORT_ERROR("TLM-2",
                            "Target does not support given generic payload transaction");
        }

        /* Processor only accepts write operations */
        if ( cmd == tlm::TLM_WRITE_COMMAND ) {
            /* Copy pixels to internal buffer */
            current_pixel = *(ptr_16_bits) & 0xFFF;
            return_pixel = *(ptr_16_bits) & 0xFFF;
        }

        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    }

    SC_CTOR(test_logger) : target_socket("target_socket")
    {
        target_socket.register_b_transport(this, &test_logger::b_transport);
    } /* End of Constructor */

}; /* End of Module counter */

int
sc_main (int argc, char* argv[])
{
    /* Module in/control */
    sc_signal<sc_uint<19> >  pixel_counter;

    /* Module out */
    sc_signal<bool>  h_sync;
    sc_signal<bool>  v_sync;
    int c_result;
    int sysc_result;

    /* Connect the DUT */
    vga_encoder video_out("video_out");
    test_logger logger("test_logger");

    logger.initiator_socket.bind(video_out.target_socket);
    video_out.initiator_socket.bind(logger.target_socket);

    /* Input/Control */
    video_out.pixel_counter(pixel_counter);

    /* Output */
    video_out.h_sync(h_sync);
    video_out.v_sync(v_sync);

    /* Initialise */
    sc_start(0, SC_NS);
    video_out.reset();
    cout << "@" << sc_time_stamp() << " Starting simulation\n" << endl;

    /* Print signals */
    uint64_t runtime = 0;
    for (runtime = 0; runtime < RUNTIME; runtime += DELAY_SEND_PIXELS) {
        /* Simulation step */
        sc_start(DELAY_SEND_PIXELS, SC_PS);
        /* Logic */
        c_result = pixel_compute ();
        logger.pixel_send(c_result);
        sysc_result = (int) logger.return_pixel;
    }

    /* Open VCD file */
    sc_trace_file *wf = sc_create_vcd_trace_file("video_encoder");
    /* Dump the desired signals */
    sc_trace(wf, c_result, "pixel_in");
    sc_trace(wf, sysc_result, "pixel_out");
    sc_trace(wf, pixel_counter, "pixel_counter");
    sc_trace(wf, h_sync, "h_sync");
    sc_trace(wf, v_sync, "v_sync");

    /* Terminate */
    cout << "@" << sc_time_stamp() << " Terminating simulation\n" << endl;
    sc_close_vcd_trace_file(wf);
    return 0;
}

/*
 * This function computes a new pixel, emulating the image-processor module
 */
sc_uint<12>
pixel_compute ()
{
    sc_uint<12> pixel;
    static sc_uint<10> col;
    static sc_uint<9> row;

    static sc_uint<4> red = 0x0;
    static sc_uint<4> green = 0X5;
    static sc_uint<4> blue = 0xA;

    /* Counters logic */
    if(col < COLS) {
        col++;
    } else {
        row++;
        col = 0;
    }
    if(row == ROWS) {
        row = 0;
    }

    /* Image builder */
    if(col % COLOR_COL_WIDTH == 0) {
        red += 0x5;
        green += 0xA;
        blue += 0xF;
    }

    pixel.range(CHANNEL_WIDTH - 1, 0) = blue;
    pixel.range(2 * CHANNEL_WIDTH - 1, CHANNEL_WIDTH) = green;
    pixel.range(3 * CHANNEL_WIDTH - 1, 2 * CHANNEL_WIDTH) = red;
    return pixel;
}

