#include "systemc.h"

#include "adc.hpp"
#include "dac.hpp"
#include "image-processor.hpp"
#include "node.hpp"
#include "router.hpp"
#include "vga_decoder.hpp"
#include "vga-encoder.hpp"

int
sc_main (int argc, char* argv[])
{
    /* Input signals */
    sc_signal<sc_uint<CHANNEL_WIDTH> > red_channel_in;
    sc_signal<sc_uint<CHANNEL_WIDTH> > green_channel_in;
    sc_signal<sc_uint<CHANNEL_WIDTH> > blue_channel_in;
    sc_signal<bool>  h_sync_in;
    sc_signal<bool>  v_sync_in;

    /* Output signals*/
    sc_signal<sc_uint<CHANNEL_WIDTH> > red_channel_out;
    sc_signal<sc_uint<CHANNEL_WIDTH> > green_channel_out;
    sc_signal<sc_uint<CHANNEL_WIDTH> > blue_channel_out;
    sc_signal<bool>  h_sync_out;
    sc_signal<bool>  v_sync_out;

    /* Nodes */
    analogic_digital_converter* adc = new analogic_digital_converter("adc");
    vga_decoder* decoder = new vga_decoder("decoder");
    Node* cpu =  new image_processor("SOBEL");
    vga_encoder* encoder = new vga_encoder("encoder");
    digital_analog_converter* dac =  new digital_analog_converter("DAC");

    /* Connect input signals to ADC */
    adc->red_channel(red_channel_in);
    adc->green_channel(green_channel_in);
    adc->blue_channel(blue_channel_in);
    decoder->h_sync(h_sync_in);
    decoder->v_sync(v_sync_in);

    /* Connect output signals to the DAC */
    dac->red_channel(red_channel_out);
    dac->green_channel(green_channel_out);
    dac->blue_channel(blue_channel_out);
    encoder->h_sync(h_sync_out);
    encoder->v_sync(v_sync_out);

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

    /* Log file */
    sc_trace_file *wf = sc_create_vcd_trace_file("top");
    sc_trace(wf, red_channel_in, "red_channel_in");
    sc_trace(wf, green_channel_in, "green_channel_in");
    sc_trace(wf, blue_channel_in, "blue_channel_in");
    sc_trace(wf, red_channel_out, "red_channel_out");
    sc_trace(wf, green_channel_out, "green_channel_out");
    sc_trace(wf, blue_channel_out, "blue_channel_out");
    sc_trace(wf, h_sync_in, "hsync_in");
    sc_trace(wf, v_sync_in, "vsync_in");
    sc_trace(wf, h_sync_out, "hsync_out");
    sc_trace(wf, v_sync_out, "vsync_out");

    /* Start the simulation, this loop emulates the sensor analogic behaviour */
    for(int i = 0; i < DEBUG_PIXELS; i++) {
        if((i % PIXELS_IN_ROW) == 0) { /* Signal new row */
            h_sync_in = 1;
        } else {
            h_sync_in = 0;
        }
        if((i % (PIXELS_IN_ROW * ROWS_IN_SCREEN)) == 0) { /* Signal new row */
            v_sync_in = 1;
        } else {
            v_sync_in = 0;
        }

        red_channel_in.write(rand() % (1 << CHANNEL_WIDTH));
        green_channel_in.write(rand() % (1 << CHANNEL_WIDTH));
        blue_channel_in.write(rand() % (1 << CHANNEL_WIDTH));
        sc_start(PIXEL_DELAY, SC_NS);
    }

    cout << "@" << sc_time_stamp() << " Terminating simulation\n" << endl;
    return 0;
}

