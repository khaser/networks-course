#include <string>
#include <tuple>
#include <iostream>
#include <random>

#include <cassert>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

const char ACK_FLAG = 32;
const char DATA_FLAG = 64;
const char STATE_MODULO = 2;
const uint32_t MSG_SIZE = 32;
const uint32_t PKG_SIZE = MSG_SIZE + 1;

class Connection {
public:
    Connection(uint32_t ip_addr, uint16_t port, uint16_t recv_port, struct timeval timeout) : state(0), addr(make_addr(ip_addr, port)), rnd(0, 1) {
        memset(buff, 0, PKG_SIZE + 1);
        sock = socket(AF_INET, SOCK_DGRAM, 0);

        auto recv_addr = make_addr(INADDR_ANY, recv_port);
        (void) bind(sock, (struct sockaddr *)&recv_addr, sizeof(recv_addr));

        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    }

    void send_noack(std::string msg) {
        send_data(msg);
        inc_state();
    }

    bool send(std::string msg) {
        send_data(msg);
        char recv_state;

        while ((recv_state = recv_ack()) == -1) {
            std::cerr << "Receive ack failed, errno: " << strerror (errno) << std::endl;
            if (errno == EAGAIN) {
                send_data(msg);
                continue;
            } else if (errno == ECONNREFUSED) {
                return false;
            } else {
                std::cerr << "Unexpected errno!\n";
                return false;
            }
        }
        assert(recv_state == (state | ACK_FLAG));
        inc_state();
        return true;
    }

    std::string recv() {
        std::string res;
        char recv_state;
        while (true) {
            std::tie(res, recv_state) = recv_data();
            if (recv_state == -1) {
                std::cerr << "Receive failed, errno: " << strerror (errno) << "\n";
                if (errno == EAGAIN) {
                    continue;
                } else if (errno == ECONNREFUSED) {
                    return "";
                } else {
                    std::cerr << "Unexpected errno, exit\n";
                    exit(1);
                }
            }
            if (recv_state != state) {
                // Ack for previous segment was loosen, so send it again
                inc_state();
                send_ack();
                inc_state();
            } else {
                // All good, just send ack for new data pkg
                send_ack();
                inc_state();
                return res;
            }
        }
    }

    ~Connection() {
        close(sock);
    }


private:

    struct sockaddr_in make_addr(uint32_t addr, uint16_t port) {
        struct sockaddr_in res;
        res.sin_family = AF_INET;
        res.sin_port = htons(port);
        res.sin_addr.s_addr = htonl(addr);
        return res;
    }

    void send_ack() {
        if (loss()) {
            buff[0] = ACK_FLAG | state;
            sendto(sock, buff, PKG_SIZE, 0, (struct sockaddr*)&addr, sizeof(addr));
        }
    }

    void send_data(std::string msg) {
        if (loss()) {
            msg = static_cast<char>(DATA_FLAG | state) + msg;
            sendto(sock, msg.data(), PKG_SIZE, 0, (struct sockaddr*)&addr, sizeof(addr));
        }
    }

    // [data, state], state == -1 if some error occurred
    std::pair<std::string, char> recv_data() {
        if (::recv(sock, buff, PKG_SIZE, 0) == -1) {
            return {"", -1};
        } else {
            return {buff + 1, buff[0] & (~DATA_FLAG)};
        }
    }

    char recv_ack() {
        if (::recv(sock, buff, PKG_SIZE, 0) == -1) {
            return -1;
        } else {
            return buff[0];
        }
    }

    void inc_state() {
        state = (state + 1) % STATE_MODULO;
    }

    bool loss() {
        return rnd(azino) > 0.3;
    }

    char buff[PKG_SIZE + 1];
    int sock;
    char state;
    struct sockaddr_in addr;
    std::mt19937 azino;
    std::uniform_real_distribution<double> rnd;
};
