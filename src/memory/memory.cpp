
#include "memory.hpp"

#define WRITE_DELAY 2

void
memory::write(sc_uint<ADDR_WIDTH> address, sc_uint<DATA_WIDTH> data)
{
    _data = data;
    _address = address;
    _wr_t.notify(WRITE_DELAY, SC_NS);
}

sc_uint<DATA_WIDTH>
memory::read(sc_uint<ADDR_WIDTH> address)
{
    _data = _ramdata [address];
    return _data;
}

void
memory::wr()
{
    while(true) {
        wait(_wr_t);
        _ramdata [_address] = _data;
    }
}
