#include <math.h>

#include "image_processor.hpp"

/* Definition of Sobel filter in horizontal direction */
const sc_int<CHANNEL_WIDTH+1> h_weights[3][3] = {
    { -1,  0,  1 },
    { -2,  0,  2 },
    { -1,  0,  1 }
};

/* Definition of Sobel filter in vertical direction */
const sc_int<CHANNEL_WIDTH+1> v_weights[3][3] = {
    { -1,  -2, -1 },
    { 0,  0,  0 },
    { 1,  2,  1 }
};

/* Interrupt Handler */
void
image_processor::frame_start()
{
    _frame_start.notify(INTERRUPT_DELAY, SC_NS);
}

static sc_uint<CHANNEL_WIDTH>
apply_sobel (sc_uint<CHANNEL_WIDTH> pixels[3][3])
{
    sc_int<2*CHANNEL_WIDTH> h_value = 0;
    sc_int<2*CHANNEL_WIDTH> v_value = 0;

    /* Get horizontal values */
    for (int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
            h_value += h_weights[i][j] * pixels[i][j];
        }
    }

    /* Get vertical values */
    for (int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
            v_value += v_weights[i][j] * pixels[i][j];
        }
    }

    /* Get sobel value */
    return sqrt(h_value*h_value + v_value*v_value);
}

static sc_uint<CHANNEL_WIDTH>
convert_to_grayscale (sc_uint<PIXEL_WIDTH> pixel)
{
    sc_uint<CHANNEL_WIDTH> gray;
    sc_uint<CHANNEL_WIDTH> r;
    sc_uint<CHANNEL_WIDTH> g;
    sc_uint<CHANNEL_WIDTH> b;

    /* Get component values from memory */
    r = pixel.range(3*CHANNEL_WIDTH - 1,
                    2*CHANNEL_WIDTH) * 0.3;
    g = pixel.range(2*CHANNEL_WIDTH - 1,
                    1*CHANNEL_WIDTH) * 0.59;
    b = pixel.range(1*CHANNEL_WIDTH - 1,
                    0) * 0.11;

    /* Get grayscale value */
    gray =  r + g + b;

    return gray;
}

void
image_processor::process()
{
    while(true) {
        wait(_frame_start);

        /* Get grayscale value for all pixels */
        for (int i = 0; i < 3; i++) {
            for( int j = 0; j < 3; j++) {
                pixels[i][j] = convert_to_grayscale (in_pixels[i + j*3]);
            }
        }

        /* Apply grayscale */
        gray = apply_sobel(pixels);

        /* Get values to output pixel */
        current_pixel.range(3*CHANNEL_WIDTH - 1, 2*CHANNEL_WIDTH) = gray;
        current_pixel.range(2*CHANNEL_WIDTH - 1, 1*CHANNEL_WIDTH) = gray;
        current_pixel.range(1*CHANNEL_WIDTH - 1, 0) = gray;

        /* Write value back*/
        pix_out.write(current_pixel);
    }
}
