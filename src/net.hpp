#include <cstdint>
#include <memory>
#include <vector>

enum net_enum {
    NET_FAIL,
    NET_SUCCESS,
    NET_TCP
};
struct net_payload {
    std::vector<char> data;
    uint64_t size = 0;

    inline void strToData(const std::string& s) {
        data.assign(s.begin(), s.end());
        size = data.size();
    }
    inline std::string dataToStr() const {
        return std::string(data.begin(), data.end());
    }
};

#define LOCALHOST "127.0.0.1"

class net_client {
    private:
    struct net_info;
    std::unique_ptr<net_info> info;

    public:
    bool connected = false;
    std::string error;

    explicit net_client(net_enum type, int protocol = 0);
    ~net_client();
    net_client(const net_client&) = delete;
    net_client& operator=(const net_client&) = delete;

    int connect(const char* ip, unsigned short port, bool throws = false);
    int send(const net_payload* data, int flags = 0, bool throws = false);
    int read(net_payload* buffer, bool throws = false);
};

class net_connection {
    public:
    int sock = -1;
    std::string error;

    explicit net_connection();
    ~net_connection();

    int send(const net_payload* data, int flags = 0, bool throws = false);
    int read(net_payload* buffer, bool throws = false);
};

class net_server {
    private:
    struct net_info;
    std::unique_ptr<net_info> info;

    public:
    bool listening = false;
    std::string error;

    explicit net_server(net_enum type, int protocol = 0);
    ~net_server();
    net_server(const net_server&) = delete;
    net_server& operator=(const net_server&) = delete;

    int listen(unsigned short port, int max_requests, bool throws = false);
    int accept(net_connection* buffer, bool throws = false);
};
