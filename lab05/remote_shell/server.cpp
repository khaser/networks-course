#include <cstring>
#include <memory>
#include <iostream>
#include "common.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 1) {
        std::cerr << "Usage: ./server\n";
        return 0;
    }

    int listener = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        throw std::logic_error("bind failed");
    }

    std::cout << "Server started" << std::endl;

    listen(listener, 1);

    while (true) {
        int sock = accept(listener, NULL, NULL);
        std::string cmd = receive(sock);
        std::cout << "Cmd to execute: " << cmd << std::endl;

        const size_t BUFF_SIZE = 1024;
        std::array<char, BUFF_SIZE+1> buff;
        buff[BUFF_SIZE] = 0;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
        while (fgets(buff.data(), BUFF_SIZE, pipe.get()) != 0) {
            int to_send = strlen(buff.data());
            send(sock, buff.data(), to_send, 0);
        }

        close(sock);
    }

    return 0;
}
