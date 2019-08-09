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

#define R_REF 1000
#define ON_V 5
#define OFF_V 0

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



#endif /* __ANALOG_4BIT_DAC_HPP__ */
