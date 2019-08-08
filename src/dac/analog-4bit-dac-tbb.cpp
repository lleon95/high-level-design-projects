
#include <systemc-ams>
#include <math.h>
#include <systemc.h>

#include "analog-4bit-dac.hpp"

SC_MODULE (digital_generator){
  /* Ports */
  sc_out<sc_uint<4> > out;

  /* Internal Variables */
  int i;
    
  void run() {
    while(true) {
      wait(10, SC_US);
      out.write(16 * sin(i++));
    }
  }

  SC_CTOR(digital_generator) {
    SC_THREAD(run)
  }
};

SC_MODULE (uint_to_double){
  /* Ports */
  sc_out<double> out;
  sc_in<sc_uint<4> > in;
    
  void do_cast() {
    out.write(in.read());
  }

  SC_CTOR(uint_to_double) {
    SC_METHOD(do_cast);
    sensitive<< in;
  }
};

int sc_main (int argc, char* argv[])

{  
	sca_eln::sca_node sigInput;
	sca_eln::sca_node sigOutput;
	sca_eln::sca_node_ref gnd;
	
	sc_signal<sc_uint<4> > uint_wire;
	sc_signal<double> double_wire;

	digital_generator generator("generator");
	generator.out(uint_wire);
	
	uint_to_double caster("cast");
	caster.in(uint_wire);
	caster.out(double_wire);
	
	sca_eln::sca_de::sca_vsource src("analog_voltage_source");
	src.set_timestep(5, sc_core::SC_US);
	src.p(sigInput);
	src.n(gnd);
	src.inp(double_wire);

	fourbit_dac dac4bit("dac4bit");

	dac4bit.vin(sigInput);
	dac4bit.vout(sigOutput);

	sca_util::sca_trace_file *eln= sca_util::sca_create_vcd_trace_file("dac4bit.vcd");
	sca_trace(eln, uint_wire, "uint_in");
	sca_trace(eln, double_wire, "double_in");
	sca_trace(eln, sigInput, "VIn");
	sca_trace(eln, sigOutput, "VOut");

	sc_start(5, sc_core::SC_MS);

	sca_util::sca_close_vcd_trace_file(eln);

	return 0;
}
