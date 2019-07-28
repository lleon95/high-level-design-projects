#include "systemc.h"

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "dac.hpp"

#define RUNTIME 390000 /* 390ns */
#define DELAY_SEND_PIXELS 39000 /* 39ns */
#define INTERRUPT_DELAY 0
#define MAX_PIXEL_OUTPUT 4096 /* 4096 = 1 << 12 */

sc_uint<PIXEL_WIDTH> pixel_compute ();

struct test_logger : sc_module {
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

    test_logger(sc_module_name test_logger) {

    } /* Constructor */

}; /* End of Module counter */

int
sc_main (int argc, char* argv[])
{
    /* Module out */
    sc_signal<sc_uint<CHANNEL_WIDTH> > red_channel;
    sc_signal<sc_uint<CHANNEL_WIDTH> > green_channel;
    sc_signal<sc_uint<CHANNEL_WIDTH> > blue_channel;
    int c_result;

    srand (time(NULL));

    /* Connect the DUT */
    dac dut_dac("dac");
    test_logger logger("test_logger");

    logger.initiator_socket.bind(dut_dac.target_socket);

    /* Output */
    dut_dac.red_channel(red_channel);
    dut_dac.green_channel(green_channel);
    dut_dac.blue_channel(blue_channel);

    /* Open VCD file */
    sc_trace_file *wf = sc_create_vcd_trace_file("dac");
    /* Dump the desired signals */
    sc_trace(wf, red_channel, "red_channel");
    sc_trace(wf, green_channel, "green_channel");
    sc_trace(wf, blue_channel, "blue_channel");
    sc_trace(wf, c_result, "pixel_in");

    /* Initialise */
    sc_start(0, SC_NS);
    cout << "@" << sc_time_stamp() << " Starting simulation\n" << endl;

    /* Print signals */
    uint64_t runtime = 0;
    for (runtime = 0; runtime < RUNTIME; runtime += DELAY_SEND_PIXELS) {
        /* Simulation step */
        sc_start(DELAY_SEND_PIXELS, SC_PS);
        /* Logic */
        c_result = pixel_compute ();
        logger.pixel_send(c_result);
    }

    /* Terminate */
    cout << "@" << sc_time_stamp() << " Terminating simulation\n" << endl;
    sc_close_vcd_trace_file(wf);
    return 0;
}

/*
 * This function computes a new pixel, emulating the image-processor module
 */
sc_uint<PIXEL_WIDTH>
pixel_compute ()
{
    sc_uint<PIXEL_WIDTH> pixel;
    pixel = rand() % MAX_PIXEL_OUTPUT; 
    return pixel;
}

