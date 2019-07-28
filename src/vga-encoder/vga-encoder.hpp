#ifndef VGA_ENCODER_HPP
#define VGA_ENCODER_HPP

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
  
  #define DESTINATION_ADDRESS 0x05

  #define WRITE_DELAY 10 /* 10ns */
  #define READ_DELAY 10 /* 10ns */
#endif

