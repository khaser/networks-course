#include "connection.h"

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

int main(int argc, char* argv[]) {

    Connection conn(INADDR_LOOPBACK, 6363, 6364, {0, 100000});
    std::ofstream fout("example_received_by_client.txt");

    std::vector<std::string> packages;
    while (true) {
        std::string s;
        s = conn.recv();
        std::cerr << "Client received: " << s << std::endl;
        fout << s;
        if (s.empty()) break;
        packages.push_back(s);
    }
    std::cerr << "File server->client received successfully\n";

    for (auto& package : packages) {
        if (!conn.send(package)) {
            std::cerr << "Connection closed unexpectedly\n";
            return 1;
        }
    }
    conn.send_noack(""); // End of file signal
    std::cerr << "File client->server received successfully\n";
    return 0;
}
