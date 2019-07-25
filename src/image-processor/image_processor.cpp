#include <math.h>

#include "image_processor.hpp"

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

/* Interrupt Handler */
void
image_processor::pixel_ready()
{
    _pixel_ready.notify(INTERRUPT_DELAY, SC_NS);
}

static sc_uint<CHANNEL_WIDTH>
apply_sobel (sc_uint<CHANNEL_WIDTH> pixel_buffer[], int pixel_index)
{
    sc_uint<CHANNEL_WIDTH> pixel_window[3][3];
    sc_int<2 * CHANNEL_WIDTH> h_value = 0;
    sc_int<2 * CHANNEL_WIDTH> v_value = 0;

    /* Copy pixels to window buffer */
    for (int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
            pixel_window[i][j] = pixel_buffer[(pixel_index + (i - 1) * WIDTH + (j - 1)) %
                                              BUFFER_SIZE];
        }
    }

    /* Get horizontal values */
    for (int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
            h_value += h_weights[i][j] * pixel_window[i][j];
        }
    }

    /* Get vertical values */
    for (int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
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
image_processor::return_pixel()
{
    tlm::tlm_generic_payload trans;
    /* Pointer to be passed to the target, target is resposible for freeing it */
    sc_uint<PIXEL_WIDTH>* return_pixel = new sc_uint<PIXEL_WIDTH>;
    sc_time delay = sc_time(INTERRUPT_DELAY, SC_NS);

    *return_pixel = current_pixel;

    trans.set_command( tlm::TLM_WRITE_COMMAND );
    trans.set_data_ptr( reinterpret_cast<unsigned char*>(return_pixel) );
    trans.set_data_length( 2 );
    trans.set_streaming_width( 2 ); /* = data_length to indicate no streaming */
    trans.set_byte_enable_ptr( 0 ); /* 0 indicates unused */
    trans.set_dmi_allowed( false ); /* Mandatory initial value */
    trans.set_response_status(
        tlm::TLM_INCOMPLETE_RESPONSE ); /* Mandatory initial value */

    printf("initiation transfer SysC: %x\t%x\n", (int)current_pixel, (int)*return_pixel);
    
    initiator_socket->b_transport( trans, delay );  // Blocking transport call
}

void
image_processor::process()
{
    while(true) {
        wait(_pixel_ready);
        /* Convert pixel to gray and save to buffer */
        pixel_buffer[pixel_index] = convert_to_grayscale( current_pixel );

        /* Apply sobel */
        gray = apply_sobel(pixel_buffer, pixel_index);

        /* Get values to output pixel */
        current_pixel.range(3 * CHANNEL_WIDTH - 1, 2 * CHANNEL_WIDTH) = gray;
        current_pixel.range(2 * CHANNEL_WIDTH - 1, 1 * CHANNEL_WIDTH) = gray;
        current_pixel.range(1 * CHANNEL_WIDTH - 1, 0) = gray;

        /* Write value back*/
        return_pixel();
	
	pixel_index = (pixel_index + 1) % BUFFER_SIZE;
    }
}

void
image_processor::b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
{
    tlm::tlm_command cmd = trans.get_command();
    unsigned char*   ptr = trans.get_data_ptr();
    unsigned int     len = trans.get_data_length();
    unsigned char*   byt = trans.get_byte_enable_ptr();
    unsigned int     wid = trans.get_streaming_width();

    short* ptr_16_bits = reinterpret_cast<short*> (ptr);


    if (byt != 0 || len > PACKAGE_LENGTH || wid < len) {
        SC_REPORT_ERROR("TLM-2",
                        "Target does not support given generic payload transaction");
    }

    /* Processor only accepts write operations */
    if ( cmd == tlm::TLM_WRITE_COMMAND ) {
      /* Copy pixels to internal buffer */
      current_pixel = *ptr_16_bits & 0xFFF;

      pixel_ready();
    }

    trans.set_response_status( tlm::TLM_OK_RESPONSE );
}
