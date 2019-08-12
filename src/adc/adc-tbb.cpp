#include "adc.hpp"

#define T (5 * PIXEL_DELAY)  // Nanoseconds. Test-signal period
#define F 1/T // Test-signal frequency. In GHz

struct dummySender : public Node {
    // Initialization done by the parent class
    dummySender(const sc_module_name & name) : Node(name)
    {
    }

    void thread_process()
    {
        for (int i = 0; i < 2; i++) { //To send two dummy transactions
            initiator->write(ADC_ADDRESS, i, tlm::TLM_WRITE_COMMAND);
            wait(BUS_DELAY, SC_NS);
        }
    }

    void reading_process()
    {
        // Not used by the simulation
    }
};

struct dummyReceiver : public Node {
    // Initialization done by the parent class
    dummyReceiver(const sc_module_name & name) : Node(name)
    {
    }

    void thread_process()
    {
        // Not used by the simulation
    }

    void reading_process()
    {
        while(true){
            wait(*(incoming_notification));
            cout << "Dummy receiver: transaction received." << endl;
        }
    }
};


int sc_main(int argc, char* argv[]){
    sc_core::sc_signal <double> input_red;
    sc_core::sc_signal <double> input_green;
    sc_core::sc_signal <double> input_blue;
    sc_core::sc_signal <short>  output_red;
    sc_core::sc_signal <short>  output_green;
    sc_core::sc_signal <short>  output_blue;
    sc_core::sc_signal <short>  output;
    sc_core::sc_signal <bool>   h_sync;
    sc_core::sc_signal <bool>   v_sync;
    
    // Instantiate the test modules
    analogicToDigitalConverter* converter = new 
        analogicToDigitalConverter("ADC");
    Node* decoder = new dummyReceiver("Decoder");
    Node* cpu = new dummySender("CPU");
    
    Router adc_router("router0", (Node *)converter);
    Router decoder_router("router1", decoder);
    Router cpu_router("router2", cpu);
    
    // Assign the router addresses
    adc_router.addr = ADC_ADDRESS;
    decoder_router.addr = DECODER_ADDRESS;
    cpu_router.addr = CPU_ADDRESS;
    
    //Connect the DUT
    converter->input_red(input_red);
    converter->input_green(input_green);
    converter->input_blue(input_blue);
    converter->output_red(output_red);
    converter->output_green(output_green);
    converter->output_blue(output_blue);
    converter->input_h_sync(h_sync);
    converter->input_v_sync(v_sync);
    
    adc_router.initiator_ring->socket.bind(decoder_router.target_ring->socket);
    decoder_router.initiator_ring->socket.bind(cpu_router.target_ring->socket);
    cpu_router.initiator_ring->socket.bind(adc_router.target_ring->socket);
    
    /* Log file */
    sca_util::sca_trace_file *tdf =
    sca_util::sca_create_vcd_trace_file("adc.vcd");
    sca_trace(tdf, input_red, "Input_RED");
    sca_trace(tdf, input_green, "Input_GREEN");
    sca_trace(tdf, input_blue, "Input_BLUE");
    sca_trace(tdf, output_red, "Output_RED");
    sca_trace(tdf, output_green, "Output_GREEN");
    sca_trace(tdf, output_blue, "Output_BLUE");
    sca_trace(tdf, output, "Output");
    sca_trace(tdf, h_sync, "H_SYNC");
    sca_trace(tdf, v_sync, "V_SYNC");
    h_sync.write(1);
    v_sync.write(1);
    sc_start(PIXEL_DELAY, SC_NS);
    h_sync.write(0);
    v_sync.write(0);
    sc_start(PIXEL_DELAY, SC_NS);
    h_sync.write(1);
    v_sync.write(1);
    
    for(int t=0; t < (5 * T) ; t ++){
        
        input_red.write((MAX_VOLTAGE / 2.00) * (sin(2 * M_PI * F * t) + 1));
        input_green.write((MAX_VOLTAGE / 2.00) * 
                          (sin((2 * M_PI * F * t) + M_PI / 2) + 1));
        input_blue.write((MAX_VOLTAGE / 2.00) * 
                         (sin((2 * M_PI * F * t) + M_PI) + 1));
        output.write(converter->getDigitalValue());
        sc_start(1, SC_NS);
    }
    sca_util::sca_close_vcd_trace_file(tdf);
    return 0;
}

