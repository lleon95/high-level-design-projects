
#include <math.h>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>

#include "image_processor.hpp"
#define TEST_ITERATIONS 10

/* Definition of Sobel filter in horizontal direction */
const int h_weights[3][3] = {
    { -1,  0,  1 },
    { -2,  0,  2 },
    { -1,  0,  1 }
};

/* Definition of Sobel filter in vertical direction */
const int v_weights[3][3] = {
    { -1,  -2, -1 },
    { 0,  0,  0 },
    { 1,  2,  1 }
};


static int
rgb12_to_gray(int pixel)
{
    int r = ((pixel & 0xF00) >> 8) * 0.3;
    int g = ((pixel & 0x0F0) >> 4) * 0.59;
    int b = ((pixel & 0x00F) >> 0) * 0.11;

    int gray = r + g + b;
    gray = gray  % ( 1 << CHANNEL_WIDTH );

    return gray;
}

static int
gray_to_sobel(int pixels[3][3])
{
    int h_value = 0;
    int v_value = 0;
    int result;

    for (int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
            h_value += h_weights[i][j] * pixels[i][j];
            v_value += v_weights[i][j] * pixels[i][j];
        }
    }

    /* Get sobel value */
    result =  sqrt(h_value * h_value + v_value * v_value);
    result = result  % ( 1 << CHANNEL_WIDTH );


    return (result << (CHANNEL_WIDTH * 2)) + (result << (CHANNEL_WIDTH * 1)) +
           result;
}


static int
process_pixel(int input_pixels, int pixel_buffer[], int index)
{

    int pixel_window[3][3];

    /* Convert pixel to gray and save to buffer */
    pixel_buffer[(index % BUFFER_SIZE)] = rgb12_to_gray( input_pixels );

    /* Copy pixels to window buffer */
    for (int i = 0; i < 3; i++) {
        for( int j = 0; j < 3; j++) {
            pixel_window[i][j] = pixel_buffer[(index + (i - 1) * WIDTH + (j - 1)) %
                                              BUFFER_SIZE];
        }
    }

    /* Apply sobel */
    return gray_to_sobel(pixel_window);
}

struct test_logger : sc_module {
    tlm_utils::simple_target_socket<test_logger> target_socket;
    tlm_utils::simple_initiator_socket<test_logger> initiator_socket;

    int return_pixel;

    void
    pixel_send(int data)
    {
        tlm::tlm_generic_payload trans;
        /* Pointer to be passed to the target, target is resposible for freeing it */
        int* return_pixel = new int;
        sc_time delay = sc_time(INTERRUPT_DELAY, SC_NS);

        *return_pixel = data;

        trans.set_command( tlm::TLM_WRITE_COMMAND );
        trans.set_data_ptr( reinterpret_cast<unsigned char*>(&data) );
        trans.set_data_length( 2 );
        trans.set_streaming_width( 2 ); /* = data_length to indicate no streaming */
        trans.set_byte_enable_ptr( 0 ); /* 0 indicates unused */
        trans.set_dmi_allowed( false ); /* Mandatory initial value */
        trans.set_response_status(
            tlm::TLM_INCOMPLETE_RESPONSE ); /* Mandatory initial value */

        initiator_socket->b_transport( trans, delay );  // Blocking transport call
    }

    /* Function to receive transfers  */
    virtual void
    b_transport(tlm::tlm_generic_payload& trans, sc_time& delay)
    {
        tlm::tlm_command cmd = trans.get_command();
        unsigned char*   ptr = trans.get_data_ptr();
        unsigned int     len = trans.get_data_length();
        unsigned char*   byt = trans.get_byte_enable_ptr();
        unsigned int     wid = trans.get_streaming_width();

        short* ptr_16_bits = reinterpret_cast<short*> (ptr);
        sc_uint<PIXEL_WIDTH> current_pixel = 0;

        if (byt != 0 || len > PACKAGE_LENGTH || wid < len) {
            SC_REPORT_ERROR("TLM-2",
                            "Target does not support given generic payload transaction");
        }

        /* Processor only accepts write operations */
        if ( cmd == tlm::TLM_WRITE_COMMAND ) {
            /* Copy pixels to internal buffer */
            current_pixel = *ptr_16_bits & 0xFFF;
            return_pixel = *ptr_16_bits & 0xFFF;
        }

        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    }

    SC_CTOR(test_logger) : target_socket("target_socket")
    {
        target_socket.register_b_transport(this, &test_logger::b_transport);
    } /* End of Constructor */

}; /* End of Module counter */


int
sc_main (int argc, char* argv[])
{
    int pixel_buffer[BUFFER_SIZE];
    int random_pixel;

    int i = 0;
    double total_error = 0;

    int c_result;
    int sysc_result;

    /* Connect the DUT */
    image_processor processor("SOBEL");
    test_logger logger("test_logger");

    for(int i = 0; i < BUFFER_SIZE; i++) {
        pixel_buffer[i] = 0;
    }

    srand (time(NULL));

    logger.initiator_socket.bind(processor.target_socket);
    processor.initiator_socket.bind(logger.target_socket);

    sc_start(1, SC_NS);

    /* Run test  */
    for (i = 0; i <  BUFFER_SIZE; i++) {
        sc_start(1, SC_NS);

        random_pixel = rand() % ( 1 << PIXEL_WIDTH );

        logger.pixel_send(random_pixel);


        sc_start(1, SC_NS);

        c_result = process_pixel(random_pixel, pixel_buffer, i);
        sysc_result = (int) logger.return_pixel;

        if(i > WIDTH) {
            total_error += abs(sysc_result - c_result);
            printf("error: %d\t", abs(sysc_result - c_result));
            printf("%d\t", i);
            printf("input pixel: %x\tsystemC output = %x\texpected output = %x\n\n",
                   random_pixel, sysc_result, c_result );
        }

    }

    cout << "Average Error: " << total_error / BUFFER_SIZE << endl;


    cout << "@" << sc_time_stamp() << " Terminating simulation\n" << endl;
    return 0; /* Terminate simulation */
}
