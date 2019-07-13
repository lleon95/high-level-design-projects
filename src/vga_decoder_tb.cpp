#include <systemc.h>
#include "vga_decoder.cpp"

#define ROWS_IN_SCREEN 525   // Rows in a screen, not the visible ones
#define FRAMES 3             // Frames to simulate
#define SIMULATION_TIME ROW_DELAY * ROWS_IN_SCREEN * FRAMES // In nano seconds

int sc_main (int argc, char* argv[]) {

  //We use srand to create a random input to be used as a pixel input.
  //Use the time stamp as the root for srand
  srand (time(NULL));
  // Inputs
  sc_signal<sc_uint<PIXEL_SIZE> > pixel_in;
  sc_signal<bool> hsync;
  sc_signal<bool> vsync;

  //Outputs
  sc_signal<sc_uint<PIXEL_SIZE> > pixel_out;

  // Instantiate the DUT
  vga_decoder decoder("Decoder");

  // Connect the DUT
  decoder.pixel_in(pixel_in);
  decoder.hsync(hsync);
  decoder.vsync(vsync);
  decoder.pixel_out(pixel_out);

  // Open VCD file
  sc_trace_file *wf = sc_create_vcd_trace_file("vga_decoder");
  wf->set_time_unit(1, SC_PS);

  // Dump the desired signals to vcd
  sc_trace(wf, pixel_in, "pixel_in");
  sc_trace(wf, hsync, "h_sync");
  sc_trace(wf, vsync, "v_sync");
  sc_trace(wf, pixel_out, "pixel_out");

  // Simulation-control variables
  int column = 1;
  int row = 1;

  //Start the simulation
  sc_start(0,SC_NS);
  cout << "@" << sc_time_stamp()<< endl; //Print the time stamp.

  // Reset the inputs
  pixel_in = 0;
  hsync = 0;
  vsync = 0;
  sc_start(10,SC_NS);

  for (double simulated_time = 0; simulated_time < SIMULATION_TIME;
    simulated_time += PIXEL_DELAY){
    if (column <= 96){  //hsync should be set.
      hsync = 1;
    }
    else{
      hsync = 0;
    }
    if (row <= 2){  //vsync should be set.
      vsync = 1;
    }
    else{
      vsync = 0;
    }
    column++;
    if (column == PIXELS_IN_ROW + 1){ //It's a new row
      column = 1;
      row++;
    }
    if (row == ROWS_IN_FRAME + 1){ //It's a new frame
      row = 1;
    }

    pixel_in.write(rand() % (1 << PIXEL_SIZE));

    sc_start(PIXEL_DELAY ,SC_NS);  // PIXEL_DELAY nano seconds elapsed
  }

  cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
  sc_close_vcd_trace_file(wf);
  return 0;// Terminate simulation

  }  //End of main
