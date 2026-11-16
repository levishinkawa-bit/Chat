#include "../net.hpp"
#include <iostream>

int main() {
    net_server server(NET_TCP);
    if (server.listen(8080, 3) == NET_FAIL) {
        std::cerr << server.error << std::endl;
        return -1;
    }
    net_connection client;

    if (server.accept(&client) == NET_FAIL) {
        std::cerr << server.error << std::endl;
        return -1;
    }
    net_payload msg;

    if (client.read(&msg) == NET_FAIL) {
        std::cerr << client.error << std::endl;
        return -1;
    }
    
    std::cout << msg.dataToStr() << std::endl;

    return 0;
}