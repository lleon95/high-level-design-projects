#include <math.h>

#include "image-processor.hpp"

/* Definition of Sobel filter in horizontal direction */
const sc_int < CHANNEL_WIDTH + 1 > h_weights[3][3] = {
    { -1,  0,  1 },
    { -2,  0,  2 },
    { -1,  0,  1 }
};

/* Definition of Sobel filter in vertical direction */
const sc_int < CHANNEL_WIDTH + 1 > v_weights[3][3] = {
    { -1,  -2, -1 },
    { 0,  0,  0 },
    { 1,  2,  1 }
};


static sc_uint<CHANNEL_WIDTH>
apply_sobel (sc_uint<CHANNEL_WIDTH> pixel_buffer[], int pixel_index)
{
    sc_uint<CHANNEL_WIDTH> pixel_window[3][3];
    sc_int<2 * CHANNEL_WIDTH> h_value = 0;
    sc_int<2 * CHANNEL_WIDTH> v_value = 0;

    /* Copy pixels to window buffer */
    for (int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
	  int index = (pixel_index + (i - 1) * WIDTH + (j - 1));
	  
	  if(index > 0 && index < BUFFER_SIZE){
            pixel_window[i][j] = pixel_buffer[index];
	  }
	  else{
	    pixel_window[i][j] = 0;
	  }
        }
    }

    /* Get vector values */
    for (int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
            h_value += h_weights[i][j] * pixel_window[i][j];
            v_value += v_weights[i][j] * pixel_window[i][j];
        }
    }

    /* Get sobel value */
    return sqrt(h_value * h_value + v_value * v_value);
}

static sc_uint<CHANNEL_WIDTH>
convert_to_grayscale (sc_uint<PIXEL_WIDTH> pixel)
{
    sc_uint<CHANNEL_WIDTH> gray;
    sc_uint<CHANNEL_WIDTH> r;
    sc_uint<CHANNEL_WIDTH> g;
    sc_uint<CHANNEL_WIDTH> b;

    /* Get component values from memory */
    r = pixel.range(3 * CHANNEL_WIDTH - 1,
                    2 * CHANNEL_WIDTH) * 0.3;
    g = pixel.range(2 * CHANNEL_WIDTH - 1,
                    1 * CHANNEL_WIDTH) * 0.59;
    b = pixel.range(1 * CHANNEL_WIDTH - 1,
                    0) * 0.11;

    /* Get grayscale value */
    gray =  r + g + b;

    return gray;
}

void
image_processor::thread_process()
{
    while(true) {
       wait(_pixel_ready);

        initiator->write(ENCODER_ADDRESS, (int)current_pixel, tlm::TLM_WRITE_COMMAND);
    }
}

void
image_processor::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        bool command = target->command;
        unsigned short data = target->incoming_buffer;
        wait(sc_time(BUS_DELAY, SC_NS));

        if(command == tlm::TLM_WRITE_COMMAND) {
            current_pixel = data & 0xFFF;

            /* Convert pixel to gray and save to buffer */
            pixel_buffer[pixel_index] = convert_to_grayscale( current_pixel );

            /* Apply sobel */
            gray = apply_sobel(pixel_buffer, pixel_index);

            /* Get values to output pixel */
            current_pixel.range(3 * CHANNEL_WIDTH - 1, 2 * CHANNEL_WIDTH) = gray;
            current_pixel.range(2 * CHANNEL_WIDTH - 1, 1 * CHANNEL_WIDTH) = gray;
            current_pixel.range(1 * CHANNEL_WIDTH - 1, 0) = gray;

            /* Write value back*/
            _pixel_ready.notify(INTERRUPT_DELAY, SC_NS);

            pixel_index = (pixel_index + 1) % BUFFER_SIZE;
        }
    }
}
