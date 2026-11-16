#if defined(_WIN32) || defined(_WIN64)
    #define PLATAFORMA_WINDOWS
#elif defined(__ANDROID__)
    #define PLATAFORMA_ANDROID
#elif defined(__linux__)
    #define PLATAFORMA_LINUX

    #include <string>
    #include <memory>
    #include <cstdint>
    #include <sys/types.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <stdexcept>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

#include "net.hpp"

struct net_client::net_info {
    int sock = -1;
    sockaddr_in serverAddr;
};

net_client::net_client(net_enum type, int protocol) {
    info = std::make_unique<net_info>();
    int prot;
    switch (type) {
        case NET_TCP:
            prot = SOCK_STREAM;
            break;
        default: throw std::invalid_argument("Invalid type");
    }
    info->sock = socket(AF_INET, prot, protocol);
    if (info->sock < 0)
        throw std::runtime_error("Failed to create socket");
}

net_client::~net_client() { close(info->sock); }

int net_client::connect(const char* ip, unsigned short port, bool throws) {
    if (connected) {
        if (throws) throw std::runtime_error("Client already connected");
        error = "Client already connected";
        return NET_FAIL;
    }
    info->serverAddr = {};
    info->serverAddr.sin_family = AF_INET;
    info->serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &info->serverAddr.sin_addr) <= 0) {
        if (throws) throw std::runtime_error("Invalid or unsupported ip");
        error = "Invalid or unsupported ip";
        return NET_FAIL;
    }

    if (::connect(info->sock, reinterpret_cast<sockaddr*>(&info->serverAddr), sizeof(info->serverAddr)) < 0) {
        if (throws) throw std::runtime_error("Failed to connect to server");
        error = "Failed to connect to server";
        return NET_FAIL;
    }

    connected = true;
    return NET_SUCCESS;
}

int net_client::send(const net_payload* payload, int flags, bool throws) {
    if (!connected) {
        if (throws) throw std::runtime_error("Not connected");
        error = "Not connected";
        return NET_FAIL;
    }
    uint64_t sent = 0;
    ssize_t sent_now;
    while (sent < sizeof(payload->size)) {
        if ((sent_now = ::send(info->sock, ((char*)&payload->size) + sent, sizeof(payload->size) - sent, flags | MSG_NOSIGNAL)) <= 0) {
            if (throws) throw std::runtime_error("Failed to send data");
            error = "Failed to send data";
            return NET_FAIL;
        }
        sent += sent_now;
    }
    sent = 0;
    while (sent < payload->size) {
        if ((sent_now = ::send(info->sock, payload->data.data() + sent, payload->size - sent, flags | MSG_NOSIGNAL)) <= 0) {
            if (throws) throw std::runtime_error("Failed to send data");
            error = "Failed to send data";
            return NET_FAIL;
        }
        sent += sent_now;
    }

    return NET_SUCCESS;
}

int net_client::read(net_payload* buffer, bool throws) {
    if (!connected) {
        if (throws) throw std::runtime_error("Not connected");
        error = "Not connected";
        return NET_FAIL;
    }
    char size_buf[sizeof(uint64_t)];
    uint64_t red = 0;
    ssize_t red_now;
    while (red < sizeof(uint64_t)) {
        if ((red_now = ::recv(info->sock, size_buf + red, sizeof(uint64_t) - red, 0)) < 0) {
            if (throws) throw std::runtime_error("Read failed");
            error = "Read failed";
            return NET_FAIL;
        }
        if (red_now == 0) {
            if (throws) throw std::runtime_error("Lost connection");
            error = "Lost connection";
            return NET_FAIL;
        }
        red += red_now;
    }
    red = 0;
    uint64_t size = *(reinterpret_cast<uint64_t*>(size_buf));
    buffer->data.resize(size);
    while (red < size) {
        if ((red_now = ::recv(info->sock, buffer->data.data() + red, size - red, 0)) < 0) {
            if (throws) throw std::runtime_error("Read failed");
            error = "Read failed";
            return NET_FAIL;
        }
        if (red_now == 0) {
            if (throws) throw std::runtime_error("Lost connection");
            error = "Lost connection";
            return NET_FAIL;
        }
        red += red_now;
    }

    return NET_SUCCESS;
}

net_connection::net_connection() {}
net_connection::~net_connection() { close(sock); }

int net_connection::send(const net_payload* payload, int flags, bool throws) {
    uint64_t sent = 0;
    ssize_t sent_now;
    while (sent < sizeof(payload->size)) {
        if ((sent_now = ::send(sock, ((char*)&payload->size) + sent, sizeof(payload->size) - sent, flags | MSG_NOSIGNAL)) <= 0) {
            if (throws) throw std::runtime_error("Failed to send data");
            error = "Failed to send data";
            return NET_FAIL;
        }
        sent += sent_now;
    }
    sent = 0;
    while (sent < payload->size) {
        if ((sent_now = ::send(sock, payload->data.data() + sent, payload->size - sent, flags | MSG_NOSIGNAL)) <= 0) {
            if (throws) throw std::runtime_error("Failed to send data");
            error = "Failed to send data";
            return NET_FAIL;
        }
        sent += sent_now;
    }

    return NET_SUCCESS;
}

int net_connection::read(net_payload *buffer, bool throws) {
    char size_buf[sizeof(uint64_t)];
    uint64_t red = 0;
    ssize_t red_now;
    while (red < sizeof(uint64_t)) {
        if ((red_now = ::recv(sock, size_buf + red, sizeof(uint64_t) - red, 0)) < 0) {
            if (throws) throw std::runtime_error("Read failed");
            error = "Read failed";
            return NET_FAIL;
        }
        if (red_now == 0) {
            if (throws) throw std::runtime_error("Lost connection");
            error = "Lost connection";
            return NET_FAIL;
        }
        red += red_now;
    }
    red = 0;
    uint64_t size = *(reinterpret_cast<uint64_t*>(size_buf));
    buffer->data.resize(size);
    while (red < size) {
        if ((red_now = ::recv(sock, buffer->data.data() + red, size - red, 0)) < 0) {
            if (throws) throw std::runtime_error("Read failed");
            error = "Read failed";
            return NET_FAIL;
        }
        if (red_now == 0) {
            if (throws) throw std::runtime_error("Lost connection");
            error = "Lost connection";
            return NET_FAIL;
        }
        red += red_now;
    }

    return NET_SUCCESS;
}

struct net_server::net_info {
    int sock = -1;
    sockaddr_in addr;
    int type;
};

net_server::net_server(net_enum type, int protocol) {
    info = std::make_unique<net_info>();
    info->type = type;
    int prot;
    switch (type) {
        case NET_TCP:
            prot = SOCK_STREAM;
            break;
        default: throw std::invalid_argument("Invalid type");
    }
    info->sock = socket(AF_INET, prot, protocol);
    if (info->sock < 0)
        throw std::runtime_error("Failed to create socket");
}

net_server::~net_server() { close(info->sock); }

int net_server::listen(unsigned short port, int max_requests, bool throws) {
    info->addr = {};
    info->addr.sin_addr.s_addr = INADDR_ANY;
    info->addr.sin_family = AF_INET;
    info->addr.sin_port = htons(port);

    if (bind(info->sock, (sockaddr*)&info->addr, sizeof(info->addr)) < 0) {
        if (throws) throw std::runtime_error("Bind failed");
        error = "Bind failed";
        return NET_FAIL;
    }
    if (::listen(info->sock, max_requests) < 0) {
        if (throws) throw std::runtime_error("Listen failed");
        error = "Listen failed";
        return NET_FAIL; 
    }

    return NET_SUCCESS;
}

int net_server::accept(net_connection *buffer, bool throws) {
    buffer->sock = ::accept(info->sock, nullptr, nullptr);
    if (buffer->sock < 0) {
        if (throws) throw std::runtime_error("Accept failed");
        error = "Accept failed";
        return NET_FAIL;
    }
    return NET_SUCCESS;
}
