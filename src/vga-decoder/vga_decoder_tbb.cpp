#include <systemc.h>
#include "vga_decoder.hpp"
#include "router.hpp"

#define FRAMES 1             // Frames to simulate
#define SIMULATION_TIME (ROW_DELAY * ROWS_IN_SCREEN * FRAMES) // In nano seconds
#define H_SYNC_SYNCH_PULSE_LENGTH 96 //In pixels
#define V_SYNC_SYNCH_PULSE_LENGTH 2  //In rows


struct adc_simulator : Node {

    adc_simulator(const sc_module_name & name) : Node(name)
    {
    } // End of Constructor

    void
    thread_process()
    {
        srand (time(NULL));
	for(int i = 0; i < (ROWS_IN_SCREEN * PIXELS_IN_ROW); i++){
	  sc_uint<PACKAGE_LENGTH_IN_BITS> data = 0;

	  data = rand() % (1 << PIXEL_WIDTH);
	  cout << "ADC sending:\t" << data << " @ " << sc_time_stamp() << endl;
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
            unsigned short data = target->incoming_buffer;

            cout << "Dummy Receiver: Transaction received: 0x" << hex
                 << data << endl;
            wait(sc_time(BUS_DELAY, SC_NS));
        }
    }
};

int
sc_main (int argc, char* argv[])
{
    /* Input signal */
    sc_signal<bool> v_sync;
    sc_signal<bool> h_sync;
  
    /* Connect the DUT */
    Node* adc =  new adc_simulator("ADC"); //
    vga_decoder* decoder = new vga_decoder("Decoder");
    Node* cpu = new DummyReceiver("CPU");

    Router adc_router("router0", adc);
    Router decoder_router("router1", decoder);
    Router cpu_router("router3", cpu);

    adc_router.initiator_ring->socket.bind(decoder_router.target_ring->socket);
    decoder_router.initiator_ring->socket.bind(cpu_router.target_ring->socket);
    cpu_router.initiator_ring->socket.bind(adc_router.target_ring->socket);

    adc_router.addr = ADC_ADDRESS;
    decoder_router.addr = DECODER_ADDRESS;
    cpu_router.addr = CPU_ADDRESS;

    decoder->h_sync(h_sync);
    decoder->v_sync(v_sync);

    sc_start(0, SC_NS);

    for(int i = 0; i < (ROWS_IN_SCREEN * PIXELS_IN_ROW); i++){
      if((i % PIXELS_IN_ROW) == 0) { /* Signal new row */
	h_sync = 1;
      }
      else {
	h_sync = 0;
      }
      if((i % (PIXELS_IN_ROW * ROWS_IN_SCREEN)) == 0) { /* Signal new row */
	v_sync = 1;
      }
      else {
	v_sync = 0;
      }
      
      sc_start(PIXEL_DELAY, SC_NS);
    }

    cout << "@" << sc_time_stamp() << " Terminating simulation" << endl;
    return 0;
}  //End of main

