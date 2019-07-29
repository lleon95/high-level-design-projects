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
        int pixel;
        for (int i = 0; i < TRANSACTIONS_TO_SEND; i++) {
            pixel = rand() % MAX_PIXEL_VALUE_PLUS_ONE;
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
    Node* encoder = new DummySender("encoder");
    encoder->addr = ENCODER_ADDRESS;
    Node* ADC =  new adc("ADC"); //
    ADC->addr = ADC_ADDRESS;
    Node* decoder = new DummyReceiver("decoder");
    decoder->addr = DECODER_ADDRESS;

    Router encoder_router("router1", encoder);
    Router adc_router("router2", ADC);
    Router decoder_router("router3", decoder);

    encoder_router.initiator_ring->socket.bind(adc_router.target_ring->socket);
    adc_router.initiator_ring->socket.bind(decoder_router.target_ring->socket);
    decoder_router.initiator_ring->socket.bind(encoder_router.target_ring->socket);

    sc_start();

    cout << "@" << sc_time_stamp() << " Terminating simulation" << endl;
    return 0;
}  //End of main

