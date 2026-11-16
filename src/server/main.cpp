#include "../net.hpp"
#include <iostream>

int main() {
    net_server server(NET_TCP);
    server.listen(8080, 3, true);
    net_connection client;

    server.accept(&client, true);
    net_payload msg;

    client.read(&msg, true);
    
    std::cout << msg.dataToStr() << std::endl;

    return 0;
}