//-----------------------------------------------------
#include "systemc.h"

#define PIXEL_SIZE 12
#define PIXEL_DELAY  39.722  //This is in nano secs
#define ROW_DELAY PIXEL_DELAY * 800 // This is in nano secs
#define UPDATE_OUTPUT_DELAY 1

SC_MODULE (vga_decoder) {
 
  //-----------Inputs/Outputs-----------------------
  
  sc_in<bool> hsync ;      // H SYNC signal
  sc_in<bool> vsync ;      // V SYNC signal
  
  sc_in<sc_uint<PIXEL_SIZE> > pixel_in;
  sc_out<sc_uint<PIXEL_SIZE> > pixel_out;
  
  //-----------Internal variables-------------------
  
  uint pixel;
  int h_count, v_count;        // Count for the column and row.
  sc_event count_column_event; //Event to notify the start of a new column          
  sc_event count_row_event;    //Event to notify the start of a new row
  sc_event update_output_event;
  
  SC_HAS_PROCESS(vga_decoder);
  // Constructor

  SC_CTOR(vga_decoder){
  
    h_count = -1;
    v_count = -1;
    pixel   =  0;
  
    SC_METHOD (decode_pixel);
    sensitive << pixel_in;
    
    SC_METHOD (start_column_count);
    sensitive << hsync.pos();
    
    SC_METHOD (start_row_count);
    sensitive << vsync.pos();
    
    SC_THREAD(column_count);
    SC_THREAD(row_count);
    SC_THREAD(update_output);
  } // End of Constructor

  //------------Code Starts Here-------------------------

  void start_column_count(){
    h_count = -1;
    count_column_event.notify();
  }

  void start_row_count(){
    v_count = -1;
    count_row_event.notify();
  }
  
  void decode_pixel() {
    if (((h_count >= 144) && (h_count < 784)) &&  //In the visible section
        ((v_count >= 35) && (v_count < 515))){
      pixel = pixel_in.read();
      update_output_event.notify(UPDATE_OUTPUT_DELAY, SC_NS);
  }
  
  void column_count() {
    while(true) {
      wait(count_column_event);
      if (h_count < 799){
        h_count += 1;
        cout<<"@" << sc_time_stamp() <<" :: Column " << h_count <<endl;
        count_column_event.notify(PIXEL_DELAY, SC_NS);
      }
    }
  }
  
  void row_count() {
    while(true) {
      wait(count_row_event);
      if (v_count < 524){
        v_count += 1;
        cout<<"@" << sc_time_stamp() <<" :: Row " << v_count <<endl;
        count_row_event.notify(ROW_DELAY, SC_NS);
      }
    }
  }
  
  void update_output() {
    while(true) {
      wait(update_output_event);
      pixel_out.write(pixel);
    }
  }

}; // End of Module vga_decoder

