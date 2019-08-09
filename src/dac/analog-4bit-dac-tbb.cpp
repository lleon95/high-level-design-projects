
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

SC_MODULE (digital_iface)
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

    SC_CTOR(digital_iface) {
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

        SC_METHOD(write_analog);
        sensitive << in;
    }

    void write_analog();
};

void
digital_iface::write_analog ()
{
    sc_uint<4> val_in = in.read();
    if(val_in[0] == 1) {
        double_wire_0.write(5);
    } else {
        double_wire_0.write(0);
    }

    if(val_in[1] == 1) {
        double_wire_1.write(5);
    } else {
        double_wire_1.write(0);
    }

    if(val_in[2] == 1) {
        double_wire_2.write(5);
    } else {
        double_wire_2.write(0);
    }

    if(val_in[3] == 1) {
        double_wire_3.write(5);
    } else {
        double_wire_3.write(0);
    }
}

int
sc_main (int argc, char* argv[])

{
    sca_eln::sca_node sigOutput;

    sc_signal<sc_uint<4> > uint_wire;

    digital_generator generator("generator");
    generator.out(uint_wire);

    /* Iface */
    digital_iface iface("iface");
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
