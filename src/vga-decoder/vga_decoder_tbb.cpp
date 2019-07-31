#include <systemc.h>
#include "vga_decoder.hpp"
#include "router.hpp"

#define ROWS_IN_SCREEN 525   // Rows in a screen, not the visible ones
#define FRAMES 1             // Frames to simulate
#define SIMULATION_TIME ROW_DELAY * ROWS_IN_SCREEN * FRAMES // In nano seconds
#define H_SYNC_SYNCH_PULSE_LENGTH 96 //In pixels
#define V_SYNC_SYNCH_PULSE_LENGTH 2  //In rows

#define DEBUG_H_SYNC_SYNCH_PULSE_LENGTH 2

struct adc_simulator : Node {

    adc_simulator(const sc_module_name & name) : Node(name)
    {
    } // End of Constructor

    void
    thread_process()
    {
        srand (time(NULL));

        int column = 0;
        int row = 0;

        bool hsync;
        bool vsync;
        int pixel = 0;
        sc_uint<PACKAGE_LENGTH_IN_BITS> data = 0;

        // Reset the variables
        hsync = 1;
        vsync = 1;
        data = rand() % (1 << PIXEL_WIDTH);
        data.range(12, 12) = hsync;
        data.range(13, 13) = vsync;
        cout << "ADC: Sending package: 0x" << hex << data << endl;
        initiator->write(DECODER_ADDRESS, (int)data, tlm::TLM_WRITE_COMMAND);
        wait(sc_time(PIXEL_DELAY, SC_NS));

#ifdef DEBUG //To be able to run the simulation fast
        for (int i = 0; i < DEBUG_PIXELS; i ++) {
            if (column <= DEBUG_H_SYNC_SYNCH_PULSE_LENGTH) { //hsync should be cleared.
                hsync = 0;
            } else {
                hsync = 1;
            }
            column++;
#else
        for (double simulated_time = 0; simulated_time < SIMULATION_TIME;
                simulated_time += PIXEL_DELAY) {
            if (column <= H_SYNC_SYNCH_PULSE_LENGHT) { //hsync should be cleared.
                hsync = 0;
            } else {
                hsync = 1;
            }
            if (row <= V_SYNC_SYNCH_PULSE_LENGHT) { //vsync should be cleared.
                vsync = 0;
            } else {
                vsync = 1;
            }
            column++;
            if (column == PIXELS_IN_ROW) { //It's a new row
                column = 0;
                row++;
		if (row == ROWS_IN_FRAME) { //It's a new frame
		  row = 0;
		}
            }
#endif //DEBUG
            data = rand() % (1 << PIXEL_WIDTH);
            data.range(12, 12) = hsync;
            data.range(13, 13) = vsync;
            cout << "ADC: Sending 0x" << hex << data << " to " << DECODER_ADDRESS
                 << " @ " << sc_time_stamp() << endl;
            initiator->write(DECODER_ADDRESS, (int)data, tlm::TLM_WRITE_COMMAND);
            wait(sc_time(PIXEL_DELAY, SC_NS));
        }
    }

    void
    reading_process()
    {
        /* No reading operations are needed on the adc_simulator */
    }
}; /* End of Module adc_simulator */

struct Memory: Node {


#ifdef DEBUG
    int received_pixels;
    unsigned short mem[DEBUG_PIXELS + 1];
#else
    enum {SIZE = RESOLUTION};
    unsigned short mem[SIZE];
#endif

    Memory(const sc_module_name & name) : Node(name)
    {
        for (int i = 0; i < sizeof(mem) / sizeof(mem[0]); i++) {
            mem[i] = 0x0;
        }
#ifdef DEBUG
        received_pixels = 0;
#endif
    } // End of Constructor

    void
    thread_process()
    {
        /* No writing operations are needed on the memory */
    }

    void
    reading_process()
    {
        while(true) {
            wait(*(incoming_notification));
            unsigned short data = target->incoming_buffer;

            cout << "CPU: Transaction received: 0x" << hex
                 << data << endl;
            mem[received_pixels] = data;
            received_pixels++;
            wait(sc_time(BUS_DELAY, SC_NS));
        }
    }
}; //End of Memory module


int
sc_main (int argc, char* argv[])
{
    /* Connect the DUT */

    Node* node_ADC =  new adc_simulator("ADC"); //
    Node* node_decoder = new vga_decoder("Decoder");
    Node* node_CPU = new Memory("CPU");

    Router adc_router("router0", node_ADC);
    Router decoder_router("router1", node_decoder);
    Router cpu_router("router3", node_CPU);

    adc_router.initiator_ring->socket.bind(decoder_router.target_ring->socket);
    decoder_router.initiator_ring->socket.bind(cpu_router.target_ring->socket);
    cpu_router.initiator_ring->socket.bind(adc_router.target_ring->socket);

    adc_router.addr = ADC_ADDRESS;
    decoder_router.addr = DECODER_ADDRESS;
    cpu_router.addr = CPU_ADDRESS;

    sc_start();
#ifdef DEBUG
    for (int i = 0;
            i < sizeof(((Memory*)node_CPU)->mem) / sizeof(((Memory*)node_CPU)->mem[0]);
            i++) {
        cout << "Memory[" << i << "] = " << hex << ((Memory*)node_CPU)->mem[i]
             << endl;
    }
#endif
    cout << "@" << sc_time_stamp() << " Terminating simulation" << endl;
    return 0;
}  //End of main

