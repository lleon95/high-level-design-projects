
#include <systemc-ams>
#include <math.h>
#include <systemc.h>

#include "analog-4bit-dac.hpp"

SC_MODULE (digital_generator)
{
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



int
sc_main (int argc, char* argv[])

{
    sca_eln::sca_node sigOutput;

    sc_signal<sc_uint<4> > uint_wire;

    digital_generator generator("generator");
    generator.out(uint_wire);

    /* Iface */
    digital_dac_interface iface("iface");
    iface.in(uint_wire);
    iface.sigOutput(sigOutput);


    sca_util::sca_trace_file *eln =
        sca_util::sca_create_vcd_trace_file("dac4bit.vcd");
    sca_trace(eln, uint_wire, "uint_in");
    sca_trace(eln, sigOutput, "VOut");

    sc_start(5, sc_core::SC_MS);

    sca_util::sca_close_vcd_trace_file(eln);

    return 0;
}
