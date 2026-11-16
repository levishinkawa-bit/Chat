#include "../../net.hpp"
#include <iostream>
#include <ostream>

int main() {
    net_client client(NET_TCP);
    if (client.connect(LOCALHOST, 8080) == NET_FAIL) {
        std::cerr << client.error << std::endl;
        return -1;
    }
    net_payload payload;
    payload.strToData("Hello world!");

    if (client.send(&payload) == NET_FAIL) {
        std::cerr << client.error << std::endl;
        return -1;
    }
    return 0;
}