#include "systemc.h"

#include "vga-encoder.hpp"
#include "vga-encoder.cpp"

#define RUNTIME 17000000000 /* 17ms */
#define RUNTIME_STEP 10000 /* 10ns */
#define CHANNEL_WIDTH 4

#define COLOR_COL_WIDTH 20

sc_uint<12> pixel_compute ();

int sc_main (int argc, char* argv[]) {
  /* Module in/control */
  sc_signal<sc_uint<12> > pixel_in;
  sc_signal<bool>  pixel_unqueue;
  sc_signal<sc_uint<19> >  pixel_counter;

  /* Module out */
  sc_signal<bool>  h_sync;
  sc_signal<bool>  v_sync;
  sc_signal<sc_uint<4> > red_channel;
  sc_signal<sc_uint<4> > green_channel;
  sc_signal<sc_uint<4> > blue_channel;

  /* Connect the DUT */
  vga_encoder video_out("video_out");
    /* Input/Control */
    video_out.pixel_in(pixel_in);
    video_out.pixel_unqueue(pixel_unqueue);
    video_out.pixel_counter(pixel_counter);
    /* Output */
    video_out.h_sync(h_sync);
    video_out.v_sync(v_sync);
    video_out.red_channel(red_channel);
    video_out.green_channel(green_channel);
    video_out.blue_channel(blue_channel);

  /* Open VCD file */
  sc_trace_file *wf = sc_create_vcd_trace_file("video_encoder");
  /* Dump the desired signals */
  sc_trace(wf, pixel_in, "pixel_in");
  sc_trace(wf, pixel_unqueue, "pixel_unqueue");
  sc_trace(wf, pixel_counter, "pixel_counter");
  sc_trace(wf, h_sync, "h_sync");
  sc_trace(wf, v_sync, "v_sync");
  sc_trace(wf, red_channel, "red_channel");
  sc_trace(wf, green_channel, "green_channel");
  sc_trace(wf, blue_channel, "blue_channel");
  
  /* Initialise */
  sc_start(0,SC_NS);
  video_out.reset();
  cout << "@" << sc_time_stamp() <<" Starting simulation\n"<< endl;

  /* Print signals */
  int runtime = 0; 
  int jump_time = RUNTIME_STEP;
  for (runtime = 0; runtime < RUNTIME; runtime += jump_time){
    /* Simulation step */
    sc_start(jump_time,SC_PS);
    /* Logic */
    if(pixel_unqueue.read()){
      jump_time = DELAY_SEND_PIXELS;
      pixel_in.write(pixel_compute ());
      video_out.write();
      video_out.read();    
      cout << "@" << sc_time_stamp() <<" Current pixel position: " 
          << pixel_counter.read() << endl;
    }
    else {
      jump_time = RUNTIME_STEP;
    }
  }

  /* Terminate */
  cout << "@" << sc_time_stamp() <<" Terminating simulation\n" << endl;
  sc_close_vcd_trace_file(wf);
  return 0;
}

sc_uint<12> pixel_compute (){
  sc_uint<12> pixel;
  static sc_uint<10>	col; 
  static sc_uint<9>	row; 

  static sc_uint<4> red = 0x0;
  static sc_uint<4> green = 0X5;
  static sc_uint<4> blue = 0xA;

  /* Counters logic */
  if(col < COLS){
    col++;
  } else {
    row++;
    col = 0;
  }
  if(row == ROWS){
    row = 0;
  }

  /* Image builder */
  if(col % COLOR_COL_WIDTH == 0){
    red += 0x5;
    green += 0xA;
    blue += 0xF;
  }

  pixel.range(CHANNEL_WIDTH - 1, 0) = blue;
  pixel.range(2*CHANNEL_WIDTH - 1, CHANNEL_WIDTH) = green;
  pixel.range(3*CHANNEL_WIDTH - 1, 2*CHANNEL_WIDTH) = red;
  return pixel;
}

