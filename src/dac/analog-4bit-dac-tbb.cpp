
#include <systemc-ams>

#include "analog-4bit-dac.hpp"

int sc_main (int argc, char* argv[])

{
	sca_eln::sca_node sigInput;
	sca_eln::sca_node sigOutput;
	sca_eln::sca_node_ref gnd;

	sca_eln::sca_vsource src("src",0.0, 0.0, 3.3, 1e3);
	src.set_timestep(5, sc_core::SC_US);

	src.p(sigInput);
	src.n(gnd);

	fourbit_dac dac4bit("dac4bit");

	dac4bit.vin(sigInput);
	dac4bit.vout(sigOutput);

	sca_util::sca_trace_file *eln= sca_util::sca_create_vcd_trace_file("dac4bit.vcd");
	sca_trace(eln, sigInput, "VIn");
	sca_trace(eln, sigOutput, "VOut");

	sc_start(5, sc_core::SC_MS);

	sca_util::sca_close_vcd_trace_file(eln);

	return 0;
}
