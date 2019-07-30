#include <systemc.h>
#include "adc.hpp"
#include "router.hpp"

#define TRANSACTIONS_TO_SEND 1

struct DummySender : public Node {
    /* Initialized in parent class */
    DummySender(const sc_module_name & name) : Node(name)
    {
    }

    void
    thread_process()
    {
        srand (time(NULL));
        int pixel = 0;
        for (int i = 0; i < TRANSACTIONS_TO_SEND; i++) {
            pixel = rand() % MAX_PIXEL_VALUE_PLUS_ONE;
            cout << "Sending 0x" << hex << pixel << " to ADC" << endl;
            initiator->write(ADC_ADDRESS, pixel, tlm::TLM_WRITE_COMMAND);
            wait(sc_time(BUS_DELAY, SC_NS));
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
    /* Connect the DUT */

    Node* node_ADC =  new analogic_digital_converter("ADC"); //
    node_ADC->addr = ADC_ADDRESS;
    Node* node_decoder = new DummyReceiver("decoder");
    node_decoder->addr = DECODER_ADDRESS;
    Node* node_encoder = new DummySender("encoder");
    node_encoder->addr = ENCODER_ADDRESS;

    Router adc_router("router0", node_ADC);
    Router decoder_router("router1", node_decoder);
    Router encoder_router("router3", node_encoder);

    adc_router.initiator_ring->socket.bind(decoder_router.target_ring->socket);
    decoder_router.initiator_ring->socket.bind(encoder_router.target_ring->socket);
    encoder_router.initiator_ring->socket.bind(adc_router.target_ring->socket);

    adc_router.addr = ADC_ADDRESS;
    decoder_router.addr = DECODER_ADDRESS;
    encoder_router.addr = ENCODER_ADDRESS;

    sc_start();
    cout << "@" << sc_time_stamp() << " Terminating simulation" << endl;
    return 0;
}  //End of main

