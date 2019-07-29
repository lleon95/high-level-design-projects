#include "systemc.h"

#include "vga-encoder.hpp"
#include "router.hpp"

#define RUNTIME 1700000000
#define CHANNEL_WIDTH 4
#define MAX_PIXEL_VALUE 4096

#define COLOR_COL_WIDTH 20

int c_result;
int sysc_result;
sc_event _pixel_ready;
sc_event _start_sim;
sc_trace_file *wf = sc_create_vcd_trace_file("decoder");

static sc_uint<12> pixel_compute ();

struct DummySender : public Node {
    /* Initialization done by the parent class */
    DummySender(const sc_module_name & name) : Node(name)
    {
    }

    void
    thread_process()
    {
        cout << "@" << sc_time_stamp() << " Starting simulation\n" << endl;

        /* Print signals */
        uint64_t runtime = 0;
        for (runtime = 0; runtime < RUNTIME; runtime += DELAY_SEND_PIXELS) {
            /* Send Logic */
            c_result = pixel_compute ();
            initiator->write(ENCODER_ADDRESS, c_result, tlm::TLM_WRITE_COMMAND);
            /* Wait until result received */
            wait(_pixel_ready);
        }

        /* Terminate */
        cout << "@" << sc_time_stamp() << " Terminating simulation\n" << endl;
        sc_close_vcd_trace_file(wf);
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
        while(true) {
            wait(*(incoming_notification));
            bool command = target->command;
            unsigned short data = target->incoming_buffer;
            wait(sc_time(BUS_DELAY, SC_NS));

            /* Transfer to the next */
            if(command == tlm::TLM_WRITE_COMMAND) {
                sysc_result = (int) data;
                _pixel_ready.notify(INTERRUPT_DELAY, SC_NS);
            }
        }
    }
};

int
sc_main (int argc, char* argv[])
{
    /* Module out */
    sc_signal<bool>  h_sync;
    sc_signal<bool>  v_sync;

    Node* cpu =  new DummySender("cpu");
    vga_encoder* encoder = new vga_encoder("decoder");
    Node* dac = new DummyReceiver("dac");

    /* Output */
    encoder->h_sync(h_sync);
    encoder->v_sync(v_sync);

    /* Connect the DUT */
    Router cpu_router("router1", cpu);
    cpu_router.addr = CPU_ADDRESS;
    Router encoder_router("router2", encoder);
    encoder_router.addr = ENCODER_ADDRESS;
    Router dac_router("router3", dac);
    dac_router.addr = DAC_ADDRESS;

    cpu_router.initiator_ring->socket.bind(encoder_router.target_ring->socket);
    encoder_router.initiator_ring->socket.bind(dac_router.target_ring->socket);
    dac_router.initiator_ring->socket.bind(cpu_router.target_ring->socket);

    srand (time(NULL));

    /* Dump the desired signals */
    sc_trace(wf, c_result, "pixel_in");
    sc_trace(wf, sysc_result, "pixel_out");
    sc_trace(wf, h_sync, "h_sync");
    sc_trace(wf, v_sync, "v_sync");

    sc_start();

    return 0;
}

/*
 * This function computes a new pixel, emulating the image-processor module
 */
static sc_uint<12>
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

