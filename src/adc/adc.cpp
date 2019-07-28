//-----------------------------------------------------

#include "adc.hpp"

//------------Code Starts Here-------------------------
void
adc::thread_process()
{
    srand (time(NULL));

    bool hsync;
    bool vsync;

    // Control variables
    int column = 1;
    int row = 1;
    short data = 0;

    // Reset the inputs
    hsync = 1;
    vsync = 1;
    data = rand() % (1 << PIXEL_SIZE);
    data = (short) (vsync << V_SYNC_POS | hsync << H_SYNC_POS | data);
    cout << "ADC: Sending 0x" << hex << data << " to " << DECODER_ADDRESS 
         << endl;
    initiator->write(DECODER_ADDRESS, (int)data, tlm::TLM_WRITE_COMMAND);
    wait(sc_time(10, SC_NS));

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
        cout << "ADC: Sending 0x" << hex << data << " to " << DECODER_ADDRESS 
             << endl;
        initiator->write(DECODER_ADDRESS, (int)data, tlm::TLM_WRITE_COMMAND);
        wait(sc_time(PIXEL_DELAY, SC_NS));   // PIXEL_DELAY nano seconds elapsed
    }
}

void
adc::reading_process()
{
    while(true){
        wait(*(incoming_notification));
        unsigned short data = target->incoming_buffer;
        //We shouldn't receive any transactions, but if we do ...
        cout << "ADC: ERROR - Transaction received: 0x" << hex << data
             << " @ "<< sc_time_stamp() << endl;
    }
}

