#ifndef VGA_ENCODER_HPP
#define VGA_ENCODER_HPP

  #include <queue>
  
  #include "systemc.h"
  #include "tlm.h"
  #include "tlm_utils/simple_initiator_socket.h"
  #include "tlm_utils/simple_target_socket.h"

  /* VGA Constants */
  #define ROWS 480
  #define COLS 640

  #define PIXEL_TIME 39.328125

  /* FSM States */
  #define FSM_VSYNC  0
  #define FSM_V_BACK_PORCH  1
  #define FSM_H_SYNC  2
  #define FSM_H_BACK_PORCH  3
  #define FSM_SEND_PIXELS  4
  #define FSM_H_FRONT_PORCH  5
  #define FSM_V_FRONT_PORCH  6
  #define FSM_IDLE  7

  /* Signal delays */
  #define DELAY_VSYNC  64000            /* 64us     */
  #define DELAY_V_BACK_PORCH  1020000   /* 1.02ms   */
  #define DELAY_H_SYNC  3770            /* 3.77us   */
  #define DELAY_H_BACK_PORCH  1890      /* 1.89us   */
  #define DELAY_SEND_PIXELS  39328      /* 39.328ns - Note this is in ps*/
  #define DELAY_H_FRONT_PORCH  940      /* 940ns    */
  #define DELAY_V_FRONT_PORCH  350000   /* 350us    */
  #define DELAY_DEFAULT 10
  #define INTERRUPT_DELAY 5

  #define NUMBER_CHANNELS_BITS 3
  #define PACKAGE_LENGTH 2 /* 16 bits package length */
  #define PIXEL_WIDTH 12
  
  #define DESTINATION_ADDRESS 0x05

  #define WRITE_DELAY 10 /* 10ns */
  #define READ_DELAY 10 /* 10ns */

struct vga_encoder : sc_module
{
    tlm_utils::simple_target_socket<vga_encoder> target_socket;
    tlm_utils::simple_initiator_socket<vga_encoder> initiator_socket;

    /* Pixels Queue */
    std::queue<unsigned short> pixels_queue;

    sc_uint<12> pixel_out;
    sc_out<sc_uint<19> >  pixel_counter;

    sc_out<bool >  h_sync;
    sc_out<bool >  v_sync;

    sc_uint<10>   col; /* 640 cols */
    sc_uint<9>    row; /* 480 rows */
    sc_uint<12> pixel;

    sc_uint<3>  state = 0;
    sc_uint<3>  next_state = 0;

    sc_event wr_t, rd_t, next_state_t, write_pixel;

    SC_HAS_PROCESS(vga_encoder);
    vga_encoder(sc_module_name vga_encoder) {

        SC_THREAD(FSM_Emulator);
        /* Ports */
        SC_THREAD(rd);

        /* Socket */
        target_socket.register_b_transport(this, &vga_encoder::b_transport);
    }

    /* Control stage */
    void FSM_Emulator();

    void FSM_next_state();

    void FSM_output_logic();

    /* Input ports */
    void reset();
    
    /* Output port - Status */
    void read();
    void rd();

    /* Datapath */
    void send_pixel();

    /* TLM implementation */
    virtual void b_transport(tlm::tlm_generic_payload& trans, sc_time& delay);

    void put_rgb_signal();

};
#endif

