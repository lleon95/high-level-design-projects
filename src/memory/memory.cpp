
#include "memory.hpp"

#define WRITE_DELAY 2

void
vga_decoder::thread_process()
{
    while(true) {
        wait(update_event);
        cout << "Memory sending:\t" << pixel << " @ " << sc_time_stamp() << endl;
        initiator->write(CPU_ADDRESS, (int)_data, tlm::TLM_WRITE_COMMAND);
        wait(sc_time(BUS_DELAY, SC_NS));
#ifdef DEBUG
        pixels_transmitted++;
#endif
    }
}

void
vga_decoder::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        bool cmd = target->command;
        unsigned short data = target->incoming_buffer;
        ap_uint<16> hw_data = data;

        cout << "Memory received:\t" << data << " @ " << sc_time_stamp() << endl;

        if (cmd == tlm::TLM_WRITE_COMMAND) {
          _ramdata[hw_data.range(15,12)] = hw_data.range(12,0);
          _data = data;
        } else {
          _data = _ramdata[hw_data.range(15,12)];
        }
        
        update_event.notify();
    }
}