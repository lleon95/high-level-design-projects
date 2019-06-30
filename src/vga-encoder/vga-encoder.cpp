#include "systemc.h"

#include "vga-encoder.hpp"

SC_MODULE (vga_encoder) {

  sc_in<sc_uint<12> > pixel_in;
  sc_out<bool >  pixel_unqueue;
  sc_out<sc_uint<18> >  pixel_counter;

  sc_out<bool >  h_sync;
  sc_out<bool >  v_sync;
  sc_out<sc_uint<4> > red_channel;
  sc_out<sc_uint<4> > green_channel;
  sc_out<sc_uint<4> > blue_channel;

  //------------Local Variables Here--------------------- 640->cols x 480-> rows
  sc_uint<10>	col; /* 640 cols */
  sc_uint<9>	row; /* 480 rows */
  sc_uint<12> pixel;

  sc_uint<3>  state = 0;
  sc_uint<3>  next_state = 0;

  sc_event wr_t, rd_t, next_state_t;

  SC_HAS_PROCESS(vga_encoder);
    vga_encoder(sc_module_name vga_encoder) {
    SC_THREAD(FSM_Emulator);
    /* Ports */
    SC_THREAD(wr);
    SC_THREAD(rd);
  }

  //------------Code Starts Here-------------------------

  /* Control stage */
  void FSM_Emulator() {
    while(true) {
      wait(next_state_t);
      state = next_state;
      FSM_next_state();
      FSM_output_logic();
    }
  }

  void FSM_next_state() {
    switch (state) {
      case FSM_VSYNC:
        next_state = FSM_V_BACK_PORCH;
        break;
      case FSM_V_BACK_PORCH:
        next_state = FSM_H_SYNC;
        break;
      case FSM_H_SYNC:
        next_state = FSM_H_BACK_PORCH;
        break;
      case FSM_H_BACK_PORCH:
        next_state = FSM_SEND_PIXELS;
        break;
      case FSM_SEND_PIXELS:
        if (col == COLS) {
          next_state = FSM_H_FRONT_PORCH;
        } else {
          next_state = FSM_SEND_PIXELS;
        }
        break;
      case FSM_H_FRONT_PORCH:
        if (row == ROWS) {
          next_state = FSM_V_FRONT_PORCH;
        } else {
          next_state = FSM_H_SYNC;
        }
        break;
      case FSM_V_FRONT_PORCH:
        next_state = FSM_VSYNC;
        break;
      default:
        next_state = FSM_VSYNC;
        break;
    }
  }

  void FSM_output_logic() {
    switch (state) {
      case FSM_VSYNC:
        v_sync.write(0);
        col = 0;
        row = 0;
        next_state_t.notify(DELAY_VSYNC,SC_NS);
        break;
      case FSM_V_BACK_PORCH:
        v_sync.write(1);
        next_state_t.notify(DELAY_V_BACK_PORCH,SC_NS);
        break;
      case FSM_V_FRONT_PORCH:
        v_sync.write(1);
        next_state_t.notify(DELAY_V_FRONT_PORCH,SC_NS);
        break;
      case FSM_H_SYNC:
        h_sync.write(0);
        col = 0;
        next_state_t.notify(DELAY_H_SYNC,SC_NS);
        break;
      case FSM_H_BACK_PORCH:
        h_sync.write(1);
        pixel_unqueue.write(0);
        next_state_t.notify(DELAY_H_BACK_PORCH,SC_NS);
        break;
      case FSM_H_FRONT_PORCH:
        h_sync.write(1);
        pixel_unqueue.write(0);
        row++;
        next_state_t.notify(DELAY_H_FRONT_PORCH,SC_NS);
        break;
      case FSM_SEND_PIXELS:
        h_sync.write(1);
        pixel_unqueue.write(1);
        //send_pixel();
        col++;
        next_state_t.notify(DELAY_SEND_PIXELS,SC_PS);
        break;
      default:
        h_sync.write(1);
        v_sync.write(1);
        next_state_t.notify(DELAY_DEFAULT,SC_NS);
        break;
    }
  }

  /* Input ports */
  void reset(){
    col = 0;
    row = 0;
    next_state = FSM_VSYNC;
    next_state_t.notify(READ_DELAY,SC_NS);
  }
  void write(){
    wr_t.notify(WRITE_DELAY, SC_NS);
  }
  void wr(){
    while(true){
      wait(wr_t);
      pixel = pixel_in.read();
      send_pixel();
      pixel_unqueue.write(0);
    }
  }

  /* Output port - Status */
  void read(){
    wr_t.notify(READ_DELAY, SC_NS);
  }
  void rd(){
    while(true){
      wait(rd_t);
      pixel_counter.write(col*row);
    }
  }

  /* Datapath */
  void send_pixel() {
    red_channel.write(pixel(11,8));
    green_channel.write(pixel(7,4));
    blue_channel.write(pixel(3,0));
  }

}; 
