
#include <stdlib.h>
#include <systemc.h>
#include <time.h>

#include "memory.hpp"

#define _1ST_VALUE 0x123
#define _2ND_VALUE 0x456
#define _3RD_VALUE 0x798
#define _4TH_VALUE 0xabc
#define _5TH_VALUE 0xdef

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
    sc_trace(wf, ram._ramdata[0x10], "data0");
    sc_trace(wf, ram._ramdata[0x11], "data1");
    sc_trace(wf, ram._ramdata[0x12], "data2");
    sc_trace(wf, ram._ramdata[0x13], "data3");
    sc_trace(wf, ram._ramdata[0x14], "data4");

    sc_start(0,SC_NS);
    cout << "@" << sc_time_stamp() << endl;
    printf("WR: addr = 0x10, data = %x\n", _1ST_VALUE);
    printf("WR: addr = 0x12, data = %x\n", _2ND_VALUE);
    printf("WR: addr = 0x13, data = %x\n", _3RD_VALUE);
    printf("WR: addr = 0x14, data = %x\n", _4TH_VALUE);
    printf("WR: addr = 0x15, data = %x\n", _5TH_VALUE);

    data = _1ST_VALUE;
    address = 0x10;
    ram.write(address, data);
    sc_start(3,SC_NS);
    data = ram.read(address);
    printf("Rd: addr = 0x10, data = %x\n", (int) data);

    data = _2ND_VALUE;
    address = 0x11;
    ram.write(address, data);

    sc_start(10,SC_NS);
    data = _3RD_VALUE;
    address = 0x12;
    ram.write(address, data);

    sc_start(10,SC_NS);
    data = _4TH_VALUE;
    address = 0x13;
    ram.write(address, data);

    sc_start(10,SC_NS);
    data = _5TH_VALUE;
    address = 0x14;
    ram.write(address, data);

    sc_start(10,SC_NS);

    address = 0x10;
    data = ram.read(address);
    sc_start(10,SC_NS);
    printf("Rd: addr = 0x10, data = %x\n",(int) data);

    address = 0x11;
    data = ram.read(address);
    sc_start(10,SC_NS);
    printf("Rd: addr = 0x11, data = %x\n",(int) data);

    address = 0x12;
    data = ram.read(address);
    sc_start(10,SC_NS);
    printf("Rd: addr = 0x12, data = %x\n",(int) data);

    address = 0x13;
    data = ram.read(address);
    sc_start(10,SC_NS);
    printf("Rd: addr = 0x13, data = %x\n",(int) data);

    address = 0x14;
    data = ram.read(address);
    sc_start(10,SC_NS);
    printf("Rd: addr = 0x13, data = %x\n",(int) data);

    cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
    sc_close_vcd_trace_file(wf);
    return 0;
}
