
#include "dummy-node.hpp"
#include "router.hpp"
#include "node_address.hpp"

int
sc_main(int argc, char* argv[])
{
    Node* node1 = new DummyNode("node1");
    Node* node2 = new DummyNode("node2");
    Node* node3 = new DummyNode("node3");
    Node* node4 = new DummyNode("node4");
    Node* node5 = new DummyNode("node5");

    Router router1("router1", node1);
    Router router2("router2", node2);
    Router router3("router3", node3);
    Router router4("router4", node4);
    Router router5("router5", node5);

    router1.addr = ADC_ADDRESS;
    router2.addr = DECODER_ADDRESS;
    router3.addr = CPU_ADDRESS;
    router4.addr = ENCODER_ADDRESS;
    router5.addr = DAC_ADDRESS;

    cout << ADC_ADDRESS << DECODER_ADDRESS << CPU_ADDRESS << ENCODER_ADDRESS <<
         DAC_ADDRESS << MAX_ADDRESS;

    /* Binding - Ring */
    router1.initiator_ring->socket.bind(router2.target_ring->socket);
    router2.initiator_ring->socket.bind(router3.target_ring->socket);
    router3.initiator_ring->socket.bind(router4.target_ring->socket);
    router4.initiator_ring->socket.bind(router5.target_ring->socket);
    router5.initiator_ring->socket.bind(router1.target_ring->socket);

    sc_start();
    return 0;
}
