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
    /* Connect the DUT */
    Node* adc = new analogic_digital_converter("adc");
    adc->addr = ADC_ADDRESS;
    Node* decoder = new vga_decoder("decoder");
    decoder->addr = ENCODER_ADDRESS;
    Node* cpu =  new image_processor("SOBEL");
    cpu->addr = CPU_ADDRESS;
    Node* encoder = new vga_encoder("encoder");
    encoder->addr = ENCODER_ADDRESS;
    Node* dac =  new digital_analog_converter("DAC"); 
    dac->addr = DAC_ADDRESS;

    Router adc_router("adc-router", adc);
    Router decoder_router("decoder-router", decoder);
    Router cpu_router("cpu-router", cpu);
    Router encoder_router("encoder-router", encoder);
    Router dac_router("dac-router", dac);

    adc_router.initiator_ring->socket.bind(decoder_router.target_ring->socket);
    decoder_router.initiator_ring->socket.bind(cpu_router.target_ring->socket);
    cpu_router.initiator_ring->socket.bind(encoder_router.target_ring->socket);
    encoder_router.initiator_ring->socket.bind(cpu_router.target_ring->socket);
    dac_router.initiator_ring->socket.bind(adc_router.target_ring->socket);
    
    cout << "@" << sc_time_stamp() << " Terminating simulation\n" << endl;
    return 0;
}

