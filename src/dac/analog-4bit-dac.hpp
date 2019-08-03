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

SC_MODULE (fourbit_dac){
  sca_eln::sca_terminal vin, vout;

  SC_CTOR(fourbit_dac): vin("vin"), vout("vout"), gnd("gnd"){
    /* First amplifier - Inverting */
    r1->p(vin);
    r1->n(vin_neg1);
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
  /* Resistors */
  sca_eln::sca_r * r1 = new sca_eln::sca_r ("r1", R_REF); 
  sca_eln::sca_r * r2 = new sca_eln::sca_r ("r2", 2*R_REF);
  sca_eln::sca_r * r3 = new sca_eln::sca_r ("r3", R_REF); 
  sca_eln::sca_r * r4 = new sca_eln::sca_r ("r4", R_REF);
  /* OpAmps */
  sca_eln::sca_nullor * opamp1 = new sca_eln::sca_nullor ("opamp1");
  sca_eln::sca_nullor * opamp2 = new sca_eln::sca_nullor ("opamp2");
};



#endif /* __ANALOG_4BIT_DAC_HPP__ */