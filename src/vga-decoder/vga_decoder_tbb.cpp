#include <systemc.h>
#include "vga_decoder.hpp"

#define ROWS_IN_SCREEN 525   // Rows in a screen, not the visible ones
#define FRAMES 1             // Frames to simulate
#define SIMULATION_TIME ROW_DELAY * ROWS_IN_SCREEN * FRAMES // In nano seconds
#define H_SYNC_SYNCH_PULSE_LENGHT 96 //In pixels
#define V_SYNC_SYNCH_PULSE_LENGHT 2  //In rows

struct adc_simulator : sc_module {
    
    tlm_utils::simple_initiator_socket<adc_simulator> initiator_socket;

    void send_data(short data)
    {
        tlm::tlm_generic_payload trans;
        sc_time delay = sc_time(TRANSACTION_DELAY, SC_NS);

        trans.set_command(tlm::TLM_WRITE_COMMAND);
        trans.set_data_ptr(reinterpret_cast<unsigned char*>(&data));
        trans.set_data_length(2);
        trans.set_streaming_width(2); /* = data_length to indicate no streaming */
        trans.set_byte_enable_ptr(0); /* 0 indicates unused */
        trans.set_dmi_allowed(false); /* Mandatory initial value */
        trans.set_response_status(
            tlm::TLM_INCOMPLETE_RESPONSE ); /* Mandatory initial value */
        cout << "ADC: Sending package: 0x" << hex << data << " @ "
        << sc_time_stamp() << endl;
        initiator_socket->b_transport(trans, delay);  // Blocking transport call
    }

    SC_CTOR(adc_simulator) : initiator_socket("initiator_socket"){} /* End of Constructor */
}; /* End of Module adc_simulator */

struct Memory: sc_module
{
    // TLM-2 socket, defaults to 32-bits wide, base protocol
    tlm_utils::simple_target_socket<Memory> target_socket;

    enum {SIZE = RESOLUTION};

    SC_CTOR(Memory) : target_socket("socket")
    {
        // Register callback for incoming b_transport interface method call
        target_socket.register_b_transport(this, &Memory::b_transport);

        // Initialize memory with random data
        for (int i = 0; i < SIZE; i++)
            mem[i] = 0x0;
    }

    // TLM-2 blocking transport method
    virtual void b_transport( tlm::tlm_generic_payload& trans, sc_time& delay )
    {
        tlm::tlm_command cmd = trans.get_command();
        sc_dt::uint64    adr = trans.get_address();
        short*             ptr = (short*) trans.get_data_ptr();
        unsigned int     len = trans.get_data_length();
        unsigned char*   byt = trans.get_byte_enable_ptr();
        unsigned int     wid = trans.get_streaming_width();

        // Obliged to check address range and check for unsupported features,
        //   i.e. byte enables, streaming, and bursts
        // Can ignore DMI hint and extensions
        // Using the SystemC report handler is an acceptable way of signalling an error

        if (adr >= sc_dt::uint64(SIZE) || byt != 0 || len > 4 || wid < len)
            SC_REPORT_ERROR("TLM-2", "Target does not support given generic payload transaction");

        // Obliged to implement read and write commands
        if ( cmd == tlm::TLM_READ_COMMAND )
            memcpy(ptr, &mem[adr], len);
        else if ( cmd == tlm::TLM_WRITE_COMMAND ){
            memcpy(&mem[adr], ptr, len);
            cout << "Memory: 0x" << hex << *ptr << " was written to 0x"
            << hex << adr << " @ " << sc_time_stamp() << endl;
        }
        // Obliged to set response status to indicate successful completion
        trans.set_response_status( tlm::TLM_OK_RESPONSE );
    }

    short mem[SIZE];
}; //End of Memory module


SC_MODULE(Top)
{
    adc_simulator *adc;
    vga_decoder   *decoder;
    Memory        *memory;

    SC_CTOR(Top)
    {
        // Instantiate components
        adc     = new adc_simulator("ADC");
        decoder = new vga_decoder("vga_decoder");
        memory  = new Memory("memory");

        adc->initiator_socket.bind(decoder->target_socket);
        decoder->initiator_socket.bind(memory->target_socket);
    }
}; //End of Top module

int
sc_main (int argc, char* argv[])
{
    srand (time(NULL));

    bool hsync;
    bool vsync;

    //Outputs
    // NONE

    // Instantiate the DUT
    Top decoder("Decoder");

    // Simulation-internal variables
    int column = 1;
    int row = 1;
    short data = 0;

    //Start the simulation
    sc_start(0,SC_NS);
    cout << "@" << sc_time_stamp()<< endl;

    // Reset the inputs
    pixel_in = 0;
    hsync = 1;
    vsync = 1;
    data = rand() % (1 << PIXEL_SIZE);
    data = (short) (vsync << V_SYNC_POS | hsync << H_SYNC_POS | data);
    decoder.adc->send_data(data);
    sc_start(10,SC_NS);

    for (double simulated_time = 0; simulated_time < SIMULATION_TIME;
            simulated_time += PIXEL_DELAY) {
        if (column <= H_SYNC_SYNCH_PULSE_LENGHT) { //hsync should be cleared.
            hsync = 0;
        } else {
            hsync = 1;
        }
        if (row <= V_SYNC_SYNCH_PULSE_LENGHT) { //vsync should be cleared.
            vsync = 0;
        } else {
            vsync = 1;
        }
        column++;
        if (column == PIXELS_IN_ROW + 1) { //It's a new row
            column = 1;
            row++;
        }
        if (row == ROWS_IN_FRAME + 1) { //It's a new frame
            row = 1;
        }
        
        data = rand() % (1 << PIXEL_SIZE);
        data = (short) (vsync << V_SYNC_POS | hsync << H_SYNC_POS | data);
        decoder.adc->send_data(data);
        sc_start(PIXEL_DELAY,SC_NS);   // PIXEL_DELAY nano seconds elapsed
    }
    cout << "@" << sc_time_stamp() <<" Terminating simulation" << endl;
    return 0;// Terminate simulation

}  //End of main

