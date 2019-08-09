/*
 * Linear Amplifier
 *
 * Created on: Ago 3rd, 2019
 *  Author: lleon
 * 
 * Version 1: Non-inverting amplifier
 */

#ifndef __ANALOG_4BIT_DAC_HPP__
#define __ANALOG_4BIT_DAC_HPP__

#include <systemc-ams>
#include <systemc.h>

#define R_REF 1000
#define ON_V 5
#define OFF_V 0
#define DAC_DELAY 1 /*1ns*/

SC_MODULE (fourbit_dac){
  sca_eln::sca_terminal vbit0, vbit1, vbit2, vbit3, vout;

  SC_CTOR(fourbit_dac): vbit0("vbit0"), vbit1("vbit1"), vbit2("vbit2") , 
      vbit3("vbit3"), vout("vout"), gnd("gnd"){
    /* First amplifier - Inverting */
    r_bit0->p(vbit0);
    r_bit1->p(vbit1);
    r_bit2->p(vbit2);
    r_bit3->p(vbit3);

    r_bit0->n(vin_neg1);
    r_bit1->n(vin_neg1);
    r_bit2->n(vin_neg1);
    r_bit3->n(vin_neg1);
    
    r2->p(vin_neg1);
    r2->n(vout_inv);
    
    opamp1->nip(gnd);
    opamp1->nin(vin_neg1);
    opamp1->nop(vout_inv);
    opamp1->non(gnd);
    
    /* Second amplifier - Deinverting */
    r3->p(vout_inv);
    r3->n(vin_neg2);
    
    r4->p(vin_neg2);
    r4->n(vout);
    
    opamp2->nip(gnd);
    opamp2->nin(vin_neg2);
    opamp2->nop(vout);
    opamp2->non(gnd);
  }
private:
  /* Nodes */
  sca_eln::sca_node_ref gnd;
  sca_eln::sca_node vin_neg1, vin_neg2, vout_inv;
  /* Bit Resistors */
  sca_eln::sca_r * r_bit0 = new sca_eln::sca_r ("r_bit0", R_REF << 3); 
  sca_eln::sca_r * r_bit1 = new sca_eln::sca_r ("r_bit1", R_REF << 2);
  sca_eln::sca_r * r_bit2 = new sca_eln::sca_r ("r_bit2", R_REF << 1); 
  sca_eln::sca_r * r_bit3 = new sca_eln::sca_r ("r_bit3", R_REF);
  /* Feedback resistors */
  // sca_eln::sca_r * r1 = new sca_eln::sca_r ("r1", R_REF); 
  sca_eln::sca_r * r2 = new sca_eln::sca_r ("r2", R_REF);
  sca_eln::sca_r * r3 = new sca_eln::sca_r ("r3", R_REF); 
  sca_eln::sca_r * r4 = new sca_eln::sca_r ("r4", R_REF);
  /* OpAmps */
  sca_eln::sca_nullor * opamp1 = new sca_eln::sca_nullor ("opamp1");
  sca_eln::sca_nullor * opamp2 = new sca_eln::sca_nullor ("opamp2");
};

SC_MODULE (digital_dac_interface)
{
    sc_in<sc_uint<4> > in;
    sca_eln::sca_terminal sigOutput;
    /* Bit signals */
    sca_eln::sca_node sigBit_0, sigBit_1, sigBit_2, sigBit_3;
    /* Output and reference*/
    sca_eln::sca_node_ref gnd;

    /* Voltage sources */
    sca_eln::sca_de::sca_vsource * src_0 = new sca_eln::sca_de::sca_vsource("avsupply_0");
    sca_eln::sca_de::sca_vsource * src_1 = new sca_eln::sca_de::sca_vsource("avsupply_1");
    sca_eln::sca_de::sca_vsource * src_2 = new sca_eln::sca_de::sca_vsource("avsupply_2");
    sca_eln::sca_de::sca_vsource * src_3 = new sca_eln::sca_de::sca_vsource("avsupply_3");

    /* Analog DAC */
    fourbit_dac * dac4bit = new fourbit_dac("dac4bit");

    /* Wires */
    sc_signal<double> double_wire_0;
    sc_signal<double> double_wire_1;
    sc_signal<double> double_wire_2;
    sc_signal<double> double_wire_3;

    SC_CTOR(digital_dac_interface) {
        /* Bits power supplies*/
        src_0->set_timestep(5, sc_core::SC_NS);
        src_0->p(sigBit_0);
        src_0->n(gnd);
        src_0->inp(double_wire_0);
	
        src_1->set_timestep(5, sc_core::SC_NS);
        src_1->p(sigBit_1);
        src_1->n(gnd);
        src_1->inp(double_wire_1);
	
        src_2->set_timestep(5, sc_core::SC_NS);
        src_2->p(sigBit_2);
        src_2->n(gnd);
        src_2->inp(double_wire_2);
	
        src_3->set_timestep(5, sc_core::SC_NS);
        src_3->p(sigBit_3);
        src_3->n(gnd);
        src_3->inp(double_wire_3);

        /* Instance */
        dac4bit->vbit0(sigBit_0);
        dac4bit->vbit1(sigBit_1);
        dac4bit->vbit2(sigBit_2);
        dac4bit->vbit3(sigBit_3);
        dac4bit->vout(sigOutput);

        SC_THREAD(write_analog);
    }

    void write_analog ()
    {
        while(true) {
          wait(DAC_DELAY, SC_NS);
          sc_uint<4> val_in = in.read();
          if(val_in[0] == 1) {
              double_wire_0.write(ON_V);
          } else {
              double_wire_0.write(OFF_V);
          }

          if(val_in[1] == 1) {
              double_wire_1.write(ON_V);
          } else {
              double_wire_1.write(OFF_V);
          }

          if(val_in[2] == 1) {
              double_wire_2.write(ON_V);
          } else {
              double_wire_2.write(OFF_V);
          }

          if(val_in[3] == 1) {
              double_wire_3.write(ON_V);
          } else {
              double_wire_3.write(OFF_V);
          }
        }
    }
};


#endif /* __ANALOG_4BIT_DAC_HPP__ */
