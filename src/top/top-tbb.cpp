#include "systemc.h"

#include "adc.hpp"
#include "dac.hpp"
#include "image-processor.hpp"
#include "node.hpp"
#include "memory.hpp"
#include "router.hpp"
#include "vga_decoder.hpp"
#include "vga-encoder.hpp"

int
sc_main (int argc, char* argv[])
{
    /* Output signals*/
    sc_signal<sc_uint<CHANNEL_WIDTH> > red_channel;
    sc_signal<sc_uint<CHANNEL_WIDTH> > green_channel;
    sc_signal<sc_uint<CHANNEL_WIDTH> > blue_channel;
    sc_signal<bool>  h_sync;
    sc_signal<bool>  v_sync;

    /* Nodes */
    Node* adc = new analogic_digital_converter("adc");
    Node* decoder = new vga_decoder("decoder");
    Node* cpu =  new image_processor("SOBEL");
    vga_encoder* encoder = new vga_encoder("encoder");
    digital_analog_converter* dac =  new digital_analog_converter("DAC");

    /* Connect output signals to the DAC */
    dac->red_channel(red_channel);
    dac->green_channel(green_channel);
    dac->blue_channel(blue_channel);
    encoder->h_sync(h_sync);
    encoder->v_sync(v_sync);


    /* Routers */
    Router adc_router("adc-router", adc);
    adc_router.addr = ADC_ADDRESS;
    Router decoder_router("decoder-router", decoder);
    decoder_router.addr = DECODER_ADDRESS;
    Router cpu_router("cpu-router", cpu);
    cpu_router.addr = CPU_ADDRESS;
    Router encoder_router("encoder-router", encoder);
    encoder_router.addr = ENCODER_ADDRESS;
    Router dac_router("dac-router", dac);
    dac_router.addr = DAC_ADDRESS;

    /* Create ring with the routers */
    adc_router.initiator_ring->socket.bind(decoder_router.target_ring->socket);
    decoder_router.initiator_ring->socket.bind(cpu_router.target_ring->socket);
    cpu_router.initiator_ring->socket.bind(encoder_router.target_ring->socket);
    encoder_router.initiator_ring->socket.bind(dac_router.target_ring->socket);
    dac_router.initiator_ring->socket.bind(adc_router.target_ring->socket);

    /* Start the simulation */
    sc_start();

    cout << "@" << sc_time_stamp() << " Terminating simulation\n" << endl;
    return 0;
}

