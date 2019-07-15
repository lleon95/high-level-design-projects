
#include <systemc.h>

/* Matches pixel width */
#define DATA_WIDTH 12
/* Number of images to store, in this case an input and an output image */
#define IMAGE_BUFFERS 2
/* Address bus width, simulator give error for bigger values */
#define ADDR_WIDTH 18
/* Pixels in an image ( 640 * 480 * IMAGE_BUFFERS ) */
#define DATA_DEPTH 1 << ADDR_WIDTH



SC_MODULE (memory)
{
    /*------------Local Variables------------------------- */
    sc_uint<DATA_WIDTH> _ramdata[DATA_DEPTH];

    sc_uint<DATA_WIDTH> _data;
    sc_uint<ADDR_WIDTH> _address;

    sc_event _wr_t;

    /*------------Module Methods---------------------------*/
    void write(sc_uint<ADDR_WIDTH> address, sc_uint<DATA_WIDTH> data);

    sc_uint<DATA_WIDTH> read(sc_uint<ADDR_WIDTH> address);

    void wr();

    SC_CTOR(memory) {
        SC_THREAD(wr);
    } /* End of Constructor */

}; /* End of memory module */
