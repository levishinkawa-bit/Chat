#include "../../net.hpp"

int main() {
    net_client client(NET_TCP);
    client.connect(LOCALHOST, 8080, true);
    net_payload payload;
    payload.strToData("Hello world!");

    client.send(&payload, true);
    return 0;
}