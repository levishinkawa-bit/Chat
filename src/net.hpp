#include <cstdint>
#include <memory>
#include <vector>

enum net_enum {
    NULO,
    FAIL,
    SUCCESS,
    TCP
};
struct net_payload {
    std::vector<char> data;
    uint64_t size = 0;
};

#define LOCALHOST "127.0.0.1"

class net_client {
    private:
    struct net_info;
    std::unique_ptr<net_info> info_p;

    public:
    bool connected = false;
    std::string error;

    explicit net_client(net_enum type, int protocol = 0);
    ~net_client();
    net_client(const net_client&) = delete;
    net_client& operator=(const net_client&) = delete;

    int connect(const char* ip, unsigned short port);
    int send(const net_payload* data, int flags = 0);
    int read(net_payload* buffer);
};
