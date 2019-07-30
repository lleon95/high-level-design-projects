//-----------------------------------------------------

#include "adc.hpp"

//------------Code Starts Here-------------------------
void
analogic_digital_converter::thread_process()
{
    srand (time(NULL));

    bool hsync;
    bool vsync;

    // Control variables
    int column = 1;
    int row = 1;
    sc_uint<PACKAGE_LENGH_IN_BITS> data = 0;

    // Reset the inputs
    hsync = 1;
    vsync = 1;
    data = rand() % MAX_PIXEL_VALUE_PLUS_ONE;
    data.range(12, 12) = hsync;
    data.range(13, 13) = vsync;

#ifdef DEBUG //To be able to run the simulation fast    
    for (int i = 0; i < DEBUG_PIXELS; i ++) {
        if (column <= DEBUG_H_SYNC_SYNCH_PULSE_LENGTH) { //hsync should be cleared.
            hsync = 0;
        } else {
            hsync = 1;
        }
        column++;

        data = rand() % (MAX_PIXEL_VALUE_PLUS_ONE);
        data.range(12, 12) = hsync;
        data.range(13, 13) = vsync;

        cout << "ADC writing:\t" << data << " @ " << sc_time_stamp() << endl;
        initiator->write(DECODER_ADDRESS, (int)data, tlm::TLM_WRITE_COMMAND);
        wait(sc_time(BUS_DELAY, SC_NS));   // PIXEL_DELAY nano seconds elapsed
    }
#else
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

        data = rand() % (MAX_PIXEL_VALUE_PLUS_ONE);
        data.range(12, 12) = hsync;
        data.range(13, 13) = vsync;

        cout << "ADC writing:\t" << data << " @ " << sc_time_stamp() << endl;
        initiator->write(DECODER_ADDRESS, (int)data, tlm::TLM_WRITE_COMMAND);
        wait(sc_time(PIXEL_DELAY, SC_NS));   // PIXEL_DELAY nano seconds elapsed
    }
#endif
}

void
analogic_digital_converter::reading_process()
{
    while(true) {
        wait(*(incoming_notification));
        unsigned short data = target->incoming_buffer;
        //We shouldn't receive any transactions, but if we do ...
        cerr << "ADC: ERROR - Transaction received: 0x" << hex << data
             << " @ " << sc_time_stamp() << endl;
    }
}

