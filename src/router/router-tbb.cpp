
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

    node1->addr = ADC_ADDRESS;
    node2->addr = DECODER_ADDRESS;
    node3->addr = CPU_ADDRESS;
    node4->addr = ENCODE_ADDRESS;
    node5->addr = DAC_ADDRESS;

    Router router1("router1", node1);
    Router router2("router2", node2);
    Router router3("router3", node3);
    Router router4("router4", node4);
    Router router5("router5", node5);

    /* Binding - Ring */
    router1.initiator_ring->socket.bind(router2.target_ring->socket);
    router2.initiator_ring->socket.bind(router3.target_ring->socket);
    router3.initiator_ring->socket.bind(router4.target_ring->socket);
    router4.initiator_ring->socket.bind(router5.target_ring->socket);
    router5.initiator_ring->socket.bind(router1.target_ring->socket);

    sc_start();
    return 0;
}
