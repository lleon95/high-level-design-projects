
#include <stdlib.h>
#include <systemc.h>
#include <time.h>

#include "memory.hpp"

#define DELAY 5

#define ADDRESS_START 0x10

const int values[5] = {0x123, 0x456, 0x789, 0xabc, 0xdef};

int
sc_main (int argc, char* argv[])
{
    sc_uint<DATA_WIDTH> data;
    sc_uint<ADDR_WIDTH> address;

    memory ram("RAM");

    /* Open VCD file */
    sc_trace_file *wf = sc_create_vcd_trace_file("memory");
    /* Dump the desired signals */
    sc_trace(wf, data, "data");
    sc_trace(wf, address, "address");
    sc_trace(wf, ram._ramdata[ADDRESS_START], "data0");
    sc_trace(wf, ram._ramdata[ADDRESS_START + 1], "data1");
    sc_trace(wf, ram._ramdata[ADDRESS_START + 2], "data2");
    sc_trace(wf, ram._ramdata[ADDRESS_START + 3], "data3");
    sc_trace(wf, ram._ramdata[ADDRESS_START + 4], "data4");

    sc_start(0,SC_NS);
    cout << "@" << sc_time_stamp() << endl;
    sc_start(DELAY,SC_NS);

    /* Write all values */
    for(int i = 0; i<5; i++) {
      data = values[i];
      address = ADDRESS_START + i;
      ram.write(address, data);
      printf("WR: addr = %x, data = %x\n", (int)address, values[i]);
      sc_start(DELAY,SC_NS);
    }
    sc_start(DELAY,SC_NS);

    printf("\n");

    for(int i = 0; i < 5; i++) {
      address = ADDRESS_START + i;
      data = ram.read(address);
      sc_start(DELAY,SC_NS);
      printf("Rd: addr = %x, data = %x\n", (int)address, (int) data);
    }

    cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
    sc_close_vcd_trace_file(wf);
    return 0;
}
