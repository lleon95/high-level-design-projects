
#include <stdlib.h>
#include <systemc.h>
#include <time.h>

#include "memory.hpp"

#define DELAY 5
#define TEST_ITERATIONS 10

#define MAX_ADDRESS DATA_DEPTH

int
sc_main (int argc, char* argv[])
{
    sc_uint<DATA_WIDTH> data;
    sc_uint<ADDR_WIDTH> address;

    int* values = (int*) malloc(sizeof(int) * TEST_ITERATIONS);
    int* addresses = (int*) malloc(sizeof(int) * TEST_ITERATIONS);

    int random_value;
    int random_address;

    memory ram("RAM");

    srand (time(NULL));

    /* Initialize with random values */
    for(int i = 0; i < TEST_ITERATIONS; i++) {
        random_value = rand() % ( 1 << DATA_WIDTH );
        random_address = rand() % (MAX_ADDRESS);

        values[i] = random_value;
        addresses[i] = random_address;
    }

    /* Open VCD file */
    sc_trace_file *wf = sc_create_vcd_trace_file("memory");
    /* Dump the desired signals */
    sc_trace(wf, data, "data");
    sc_trace(wf, address, "address");

    for(int i = 0; i < TEST_ITERATIONS; i++) {
        std::string field_name = "data";
        field_name += std::to_string(i);

        sc_trace(wf, ram._ramdata[addresses[i]], field_name);
    }

    sc_start(0,SC_NS);
    cout << "@" << sc_time_stamp() << endl;
    sc_start(DELAY,SC_NS);

    /* Write all values */
    for(int i = 0; i < TEST_ITERATIONS; i++) {
        data = values[i];
        address = addresses[i];

        ram.write(address, data);
        printf("WR: addr = %x, data = %x\n", (int)address, (int)data);
        sc_start(DELAY,SC_NS);
    }
    sc_start(DELAY,SC_NS);

    printf("\n");

    for(int i = 0; i < TEST_ITERATIONS; i++) {
        address = addresses[i];

        data = ram.read(address);
        sc_start(DELAY,SC_NS);
        printf("Rd: addr = %x, data = %x\n", (int)address, (int) data);
    }

    cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
    sc_close_vcd_trace_file(wf);
    return 0;
}
