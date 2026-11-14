#if defined(_WIN32) || defined(_WIN64)
    #define PLATAFORMA_WINDOWS
#elif defined(__ANDROID__)
    #define PLATAFORMA_ANDROID
#elif defined(__linux__)
    #define PLATAFORMA_LINUX

    #include <cstdint>
    #include <sys/types.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <exception>
    #include <stdexcept>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

#include "net.hpp"

class net_error : public std::exception {
    const char* text;
    public:
    net_error(const char* text): text(text) {}
    const char * what() const noexcept override {
        return text;
    }
};

struct net_client::net_info {
    int sock = -1;
    sockaddr_in serverAddr;
    int type;
};

net_client::net_client(net_enum type, int protocol) {
    info_p->type = type;
    int prot;
    switch (type) {
        case TCP:
            prot = SOCK_STREAM;
            break;
        default: throw std::invalid_argument("Invalid type");
    }
    info_p->sock = socket(AF_INET, prot, protocol);
    if (info_p->sock < 0)
        throw net_error("Failed to create socket");
}

net_client::~net_client() {
    close(info_p->sock);
}

int net_client::connect(const char* ip, unsigned short port) {
    if (connected) {
        error = "Client already connected";
        return FAIL;
    }
    info_p->serverAddr.sin_family = AF_INET;
    info_p->serverAddr.sin_port = port;

    if (inet_pton(AF_INET, ip, &info_p->serverAddr) < 0) {
        error = "Invalid or unsupported ip";
        return FAIL;
    }

    if (::connect(info_p->sock, reinterpret_cast<sockaddr*>(&info_p->serverAddr), sizeof(info_p->serverAddr)) < 0) {
        error = "Failed to connect to server";
        return FAIL;
    }

    connected = true;
    return SUCCESS;
}

int net_client::send(const net_payload* payload, int flags) {
    uint64_t sent = 0;
    ssize_t sent_now;
    while (sent < sizeof(payload->size)) {
        if ((sent_now = ::send(info_p->sock, ((char*)&payload->size) + sent, sizeof(payload->size) - sent, flags)) <= 0) {
            error = "Failed to send data";
            return FAIL;
        }
        sent += sent_now;
    }
    sent = 0;
    while (sent < payload->size) {
        if ((sent_now = ::send(info_p->sock, payload->data.data() + sent, payload->size - sent, flags)) <= 0) {
            error = "Failed to send data";
            return FAIL;
        }
        sent += sent_now;
    }

    return SUCCESS;
}

int net_client::read(net_payload* buffer) {
    char size_buf[sizeof(uint64_t)];
    uint64_t red = 0;
    ssize_t red_now;
    while (red < sizeof(uint64_t)) {
        if ((red_now = ::recv(info_p->sock, size_buf + red, sizeof(uint64_t) - red, 0)) < 0) {
            error = "Read failed";
            return FAIL;
        }
        if (red_now == 0) {
            error = "Lost connection";
            return FAIL;
        }
        red += red_now;
    }
    red = 0;
    uint64_t size = *(reinterpret_cast<uint64_t*>(size_buf));
    buffer->data.resize(size);
    while (red < size) {
        if ((red_now = ::recv(info_p->sock, buffer->data.data() + red, size - red, 0)) < 0) {
            error = "Read failed";
            return FAIL;
        }
        if (red_now == 0) {
            error = "Lost connection";
            return FAIL;
        }
        red += red_now;
    }

    return SUCCESS;
}
