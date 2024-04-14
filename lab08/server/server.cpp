#include "connection.h"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

int main(int argc, char* argv[]) {

    std::ifstream fin("example.txt");
    std::vector<std::string> packages;
    char buff[MSG_SIZE];
    std::streamsize readed;
    while ((readed = fin.readsome(buff, MSG_SIZE))) {
        packages.emplace_back(buff, readed);
    }
    std::cerr << "File divided into " << packages.size() << " parts\n";

    Connection conn(INADDR_LOOPBACK, 6364, 6363, {0, 100000});
    for (auto& package : packages) {
        if (!conn.send(package)) {
            std::cerr << "Connection closed unexpectedly\n";
            return 1;
        }
    }

    conn.send_noack(""); // End of file signal
    std::cerr << "File server->client transmitted successfully\n";

    std::ofstream fout("example_received_by_server.txt");
    while (true) {
        std::string s;
        s = conn.recv();
        std::cerr << "Server received: " << s << std::endl;
        fout << s;
        if (s.empty()) break;
        packages.push_back(s);
    }
    std::cerr << "File client->server received successfully\n";
    return 0;
}
