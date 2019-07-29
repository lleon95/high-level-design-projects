#include "systemc.h"

#include "tlm.h"
#include "tlm_utils/simple_initiator_socket.h"
#include "tlm_utils/simple_target_socket.h"

#include "dac.hpp"
#include "router.hpp"

#define RUNTIME 390000 /* 390ns */
#define DELAY_SEND_PIXELS 39000 /* 39ns */
#define MAX_PIXEL_OUTPUT 4096 /* 4096 = 1 << 12 */

sc_uint<PIXEL_WIDTH> pixel_compute ();

int c_result;
sc_event * dut_wr_t;

struct DummySender : public Node {
    /* Initialization done by the parent class */
    DummySender(const sc_module_name & name) : Node(name)
    {
    }

    void
    thread_process()
    {
        /* Print signals */
        uint64_t runtime = 0;
        for (runtime = 0; runtime < RUNTIME; runtime += DELAY_SEND_PIXELS) {
            /* Send Logic */
            c_result = pixel_compute ();
            initiator->write(DAC_ADDRESS, c_result, tlm::TLM_WRITE_COMMAND);
            /* Wait until result received */
            wait(DELAY_SEND_PIXELS, SC_PS);
        }
    }

    void
    reading_process()
    {
        /* No reading operations are needed on the dummy sender */
    }
};

struct DummyReceiver : public Node {
    /* Initialization done by the parent class */
    DummyReceiver(const sc_module_name & name) : Node(name)
    {
    }

    void
    thread_process()
    {
        /* No writing operations are needed on the dummy receiver */
    }

    void
    reading_process()
    {
        /* No reading operations, just exist by the moment */
    }
};

int
sc_main (int argc, char* argv[])
{
    sc_signal<sc_uint<CHANNEL_WIDTH> > red_channel;
    sc_signal<sc_uint<CHANNEL_WIDTH> > green_channel;
    sc_signal<sc_uint<CHANNEL_WIDTH> > blue_channel;

    Node* encoder = new DummySender("encoder");
    dac* dut_dac =  new dac("DAC"); //
    Node* decoder = new DummyReceiver("decoder");

    /* Output */
    dut_dac->red_channel(red_channel);
    dut_dac->green_channel(green_channel);
    dut_dac->blue_channel(blue_channel);
    dut_wr_t = &(dut_dac->wr_t);

    /* Log file */
    sc_trace_file *wf = sc_create_vcd_trace_file("dac");
    sc_trace(wf, red_channel, "red_channel");
    sc_trace(wf, green_channel, "green_channel");
    sc_trace(wf, blue_channel, "blue_channel");
    sc_trace(wf, c_result, "pixel_in");

    /* Connect the DUT */
    Router encoder_router("router1", encoder);
    Router dac_router("router2", dut_dac);
    Router decoder_router("router3", decoder);
    encoder_router.addr = ENCODER_ADDRESS;
    dac_router.addr = DAC_ADDRESS;
    decoder_router.addr = ADC_ADDRESS;

    encoder_router.initiator_ring->socket.bind(dac_router.target_ring->socket);
    dac_router.initiator_ring->socket.bind(decoder_router.target_ring->socket);
    decoder_router.initiator_ring->socket.bind(encoder_router.target_ring->socket);

    srand (time(NULL));

    cout << "@" << sc_time_stamp() << " Starting simulation\n" << endl;
    sc_start();

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

