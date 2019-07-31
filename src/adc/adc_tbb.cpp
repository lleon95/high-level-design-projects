#include <systemc.h>
#include "adc.hpp"
#include "router.hpp"

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
    sc_signal<sc_uint<CHANNEL_WIDTH> > red_channel;
    sc_signal<sc_uint<CHANNEL_WIDTH> > green_channel;
    sc_signal<sc_uint<CHANNEL_WIDTH> > blue_channel;

    /* Connect the DUT */
    analogic_digital_converter* node_ADC =  new
    analogic_digital_converter("ADC"); //
    Node* node_decoder = new DummyReceiver("decoder");

    node_ADC->red_channel(red_channel);
    node_ADC->green_channel(green_channel);
    node_ADC->blue_channel(blue_channel);

    Router adc_router("router0", node_ADC);
    Router decoder_router("router1", node_decoder);

    srand (time(NULL));

    adc_router.initiator_ring->socket.bind(decoder_router.target_ring->socket);
    decoder_router.initiator_ring->socket.bind(adc_router.target_ring->socket);

    adc_router.addr = ADC_ADDRESS;
    decoder_router.addr = DECODER_ADDRESS;

    for(int i = 0; i < DEBUG_PIXELS; i++) {
        red_channel.write(rand() % (1 << CHANNEL_WIDTH));
        green_channel.write(rand() % (1 << CHANNEL_WIDTH));
        blue_channel.write(rand() % (1 << CHANNEL_WIDTH));
	cout << "changed value@ " << sc_time_stamp()  << endl;
        sc_start(PIXEL_DELAY, SC_NS);
    }

    cout << "@" << sc_time_stamp() << " Terminating simulation" << endl;
    return 0;
}  //End of main

