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
    sc_core::sc_signal <double> input_red;
    sc_core::sc_signal <double> input_green;
    sc_core::sc_signal <double> input_blue;
    sc_signal<bool>  h_sync_in;
    sc_signal<bool>  v_sync_in;

    /* ADC output*/
    sc_core::sc_signal <short>  output_red;
    sc_core::sc_signal <short>  output_green;
    sc_core::sc_signal <short>  output_blue;

    /* Output signals*/
    sca_eln::sca_node red_channel_out;
    sca_eln::sca_node green_channel_out;
    sca_eln::sca_node blue_channel_out;
    sc_signal<bool>  h_sync_out;
    sc_signal<bool>  v_sync_out;

    /* Nodes */
    analogic_digital_converter* adc = new analogic_digital_converter("adc");
    vga_decoder* decoder = new vga_decoder("decoder");
    Node* cpu =  new image_processor("SOBEL");
    vga_encoder* encoder = new vga_encoder("encoder");
    digital_analog_converter* dac =  new digital_analog_converter("DAC");

    /* Connect input signals to ADC */
    adc->input_red(input_red);
    adc->input_green(input_green);
    adc->input_blue(input_blue);
    adc->output_red(output_red);
    adc->output_green(output_green);
    adc->output_blue(output_blue);
    
    decoder->h_sync(h_sync_in);
    decoder->v_sync(v_sync_in);

    /* Connect output signals to the DAC */
    dac->r_channel(red_channel_out);
    dac->g_channel(green_channel_out);
    dac->b_channel(blue_channel_out);
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
    sca_util::sca_trace_file *wf = sca_util::sca_create_vcd_trace_file("top");
    sca_trace(wf, input_red, "red_channel_in");
    sca_trace(wf, input_green, "green_channel_in");
    sca_trace(wf, input_blue, "blue_channel_in");
    sca_trace(wf, red_channel_out, "red_channel_out");
    sca_trace(wf, green_channel_out, "green_channel_out");
    sca_trace(wf, blue_channel_out, "blue_channel_out");
    sca_trace(wf, h_sync_in, "hsync_in");
    sca_trace(wf, v_sync_in, "vsync_in");
    sca_trace(wf, h_sync_out, "hsync_out");
    sca_trace(wf, v_sync_out, "vsync_out");

    /* Start the simulation, this loop emulates the sensor analogic behaviour */
    for(int i = 0; i < ROWS_IN_SCREEN; i++) {
      for(int j = 0; j < PIXELS_IN_ROW; j++) {
        if(j == 0) { /* Signal new row */
            h_sync_in = 1;
        } else {
            h_sync_in = 0;
        }
        if(i == 0) { /* Signal new row */
            v_sync_in = 1;
        } else {
            v_sync_in = 0;
        }

	if((i > 250) && (i < 350 ) && (j >  350) && (j < 450)){
	  input_red.write(MAX_VOLTAGE);
	  input_green.write(MAX_VOLTAGE);
	  input_blue.write(MAX_VOLTAGE);
	}
	else {
	  input_red.write(0);
	  input_green.write(0);
	  input_blue.write(0);
	}
        sc_start(PIXEL_DELAY, SC_NS);
      }
    }
    cout << "@" << sc_time_stamp() << " Terminating simulation\n" << endl;

    
    sca_util::sca_close_vcd_trace_file(wf);
    return 0;
}

