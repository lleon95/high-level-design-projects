
#include <math.h>
#include <stdlib.h>
#include <systemc.h>
#include <time.h>

#include "image-processor.hpp"
#include "router.hpp"

#define TEST_ITERATIONS 10

int test_pixel;
int pixel_buffer[BUFFER_SIZE];
int c_result;
int sysc_result;
double total_error = 0;
sc_event _pixel_ready;

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
process_pixel(int input_pixels, int pixel_buffer[], int pixel_index)
{

  int pixel_window[3][3];

  /* Convert pixel to gray and save to buffer */
  pixel_buffer[pixel_index] = rgb12_to_gray( input_pixels );

  /* Copy pixels to window buffer */
  for (int i = 0; i < 3; i++) {
    for( int j = 0; j < 3; j++) {
      int index = (pixel_index + (i - 2) * WIDTH + (j - 2));
      if(index < BUFFER_SIZE && index >= 0) {
	pixel_window[i][j] = pixel_buffer[index];
      }
      else if (index < 0) {
	index += BUFFER_SIZE;
	pixel_window[i][j] = pixel_buffer[index];
      }
      else {
	pixel_window[i][j] = 0;
      }
    }
  }

  /* Apply sobel */
  return gray_to_sobel(pixel_window);
}

struct DummySender : public Node {
    /* Initialization done by the parent class */
    DummySender(const sc_module_name & name) : Node(name)
    {
    }

    void
    thread_process()
    {
        int pixel_error;
	int index ;
        for (int j = 0; j < HEIGHT; j++) {
            for (int i = 0; i < WIDTH; i++) {
	      if(j > (HEIGHT/2)){
		test_pixel = (1 << PIXEL_WIDTH) - 1;
	      }

#ifdef TRANSACTION_PRINT
                cout << "DummySender sending:\t" << test_pixel << " @ " << sc_time_stamp() <<
                     endl;
#endif /* TRANSACTION_PRINT */
                initiator->write(CPU_ADDRESS, test_pixel, tlm::TLM_WRITE_COMMAND);

                /* Don't process another pixel until result can be tested */
                wait(_pixel_ready);

                c_result = process_pixel(test_pixel, pixel_buffer,
                                         (i + j * WIDTH) % BUFFER_SIZE);

                pixel_error = abs(c_result - sysc_result);

                total_error += pixel_error;
		//#ifdef DEBUG
                printf("error: %d\t", pixel_error);
                printf("(%d, %d)\t", i, j);
                printf("input_pixel: %x\t", test_pixel);
                printf("systemC_output: %x\t", sysc_result);
                printf("expected output: %x\n", c_result);
		//#endif /* DEBUG */

            }
        }
        printf("Average Error: %f\n", total_error / (WIDTH * HEIGHT));
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
            bool command = target->command;
            unsigned short data = target->incoming_buffer;
#ifdef TRANSACTION_PRINT
            cout << "DummyReceiver received:\t" << data << " @ " << sc_time_stamp() << endl;
#endif /* TRANSACTION_PRINT */
            /* Transfer to the next */
            if(command == tlm::TLM_WRITE_COMMAND) {
                sysc_result = (int) data;
                _pixel_ready.notify(INTERRUPT_DELAY, SC_NS);
            }
        }
    }
};

int
sc_main (int argc, char* argv[])
{
    /* Connect the DUT */
    Node* decoder = new DummySender("decoder");
    Node* cpu =  new image_processor("SOBEL"); //
    Node* encoder = new DummyReceiver("encoder");

    Router decoder_router("decoder_router", decoder);
    Router cpu_router("cpu_router", cpu);
    Router encoder_router("encoder_router", encoder);

    decoder_router.addr = DECODER_ADDRESS;
    cpu_router.addr = CPU_ADDRESS;
    encoder_router.addr = ENCODER_ADDRESS;

    cout << decoder_router.addr << endl;
    cout << cpu_router.addr << endl;
    cout << encoder_router.addr << endl;

    decoder_router.initiator_ring->socket.bind(cpu_router.target_ring->socket);
    cpu_router.initiator_ring->socket.bind(encoder_router.target_ring->socket);
    encoder_router.initiator_ring->socket.bind(decoder_router.target_ring->socket);

    for(int i = 0; i < BUFFER_SIZE; i++) {
        pixel_buffer[i] = 0;
    }

    srand (time(NULL));

    sc_start();

    cout << "Average Error: " << total_error / BUFFER_SIZE << endl;

    cout << "@" << sc_time_stamp() << " Terminating simulation\n" << endl;
    return 0; /* Terminate simulation */
}
