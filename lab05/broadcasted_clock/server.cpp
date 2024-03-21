#include <iostream>
#include <chrono>
#include <thread>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

using namespace std::chrono_literals;

int main(int argc, char* argv[]) {
    if (argc != 1) {
        std::cerr << "Usage: ./server\n";
        return 0;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    int YES = 1;
    int err = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &YES, sizeof(YES));
    if (err) {
        return err;
    }

    std::cout << "Server started" << std::endl;
    while (true) {
        using namespace std::chrono;
        std::ostringstream ss;
        ss << system_clock::now() << std::endl;
        std::string cur_time = ss.str();
        std::cout << "Broadcast time: " << cur_time;

        sendto(sock, cur_time.c_str(), sizeof(char) * cur_time.length(), 0, (struct sockaddr*)&addr, sizeof(addr));

        std::this_thread::sleep_for(1s);
    }


    return 0;
}
