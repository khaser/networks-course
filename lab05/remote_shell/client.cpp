#include <cstring>
#include <iostream>
#include <string>
#include "common.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./client \"command to run on server\"\n";
        return 0;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    (void) connect(sock, (struct sockaddr *)&addr, sizeof(addr));

    send(sock, argv[1], strlen(argv[1]), 0);

    std::cout << "Command output:\n" << receive(sock, true) << std::endl;

    close(sock);
    return 0;
}
