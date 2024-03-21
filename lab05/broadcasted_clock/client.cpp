#include <iostream>
#include <string>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

const size_t BUFF_SIZE = 1024;

int main(int argc, char* argv[]) {
    if (argc != 1) {
        std::cerr << "Usage: ./client\n";
        return 0;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int YES = 1;
    int err = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &YES, sizeof(YES));
    if (err) {
        return err;
    }

    (void) bind(sock, (struct sockaddr *)&addr, sizeof(addr));

    std::cout << "Client started" << std::endl;
    while (true) {
        char buff[BUFF_SIZE];
        recvfrom(sock, buff, sizeof(buff), 0, NULL, NULL);
        std::cout << "Received: " << std::string(buff) << std::endl;
    }


    return 0;
}
