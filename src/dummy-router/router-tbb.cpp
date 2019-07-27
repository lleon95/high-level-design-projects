
#include "router.hpp"

int
sc_main(int argc, char* argv[])
{
    Node node1("node1");
    Node node2("node2");
    Node node3("node3");
    Node node4("node4");
    Node node5("node5");

    node1.addr = 0;
    node2.addr = 1;
    node3.addr = 2;
    node4.addr = 3;
    node5.addr = 4;

    Router router1("router1", &node1);
    Router router2("router2", &node2);
    Router router3("router3", &node3);
    Router router4("router4", &node4);
    Router router5("router5", &node5);

    /* Binding - Ring */
    router1.initiator_ring->socket.bind(router2.target_ring->socket);
    router2.initiator_ring->socket.bind(router3.target_ring->socket);
    router3.initiator_ring->socket.bind(router4.target_ring->socket);
    router4.initiator_ring->socket.bind(router5.target_ring->socket);
    router5.initiator_ring->socket.bind(router1.target_ring->socket);

    sc_start();
    return 0;
}
