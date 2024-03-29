#include <iostream>
#include <string>
#include <array>
#include <sstream>
#include <fstream>
#include <limits>
#include <vector>
#include <chrono>
#include <thread>

#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>

/* #define DEBUG 1 */

class Connection {
protected:
    std::string ask(int sock, std::string msg) {
        send(sock, msg);
        return receive(sock);
    }

    std::string receive(int sock, bool wait=false) {
        std::string acc;
        {
            std::array<char, BUFF_SIZE+1> buff;
            int bytes_read;
            do {
                bytes_read = recv(sock, buff.data(), BUFF_SIZE, 0);
                buff[bytes_read] = 0;
                if (bytes_read <= 0) break;
                acc += std::string(buff.data());
            } while (bytes_read == BUFF_SIZE || wait);
        }
#ifdef DEBUG
        std::cerr << "Received from server: " << acc << std::endl;
#endif
        return acc;
    }

    void send(int sock, std::ifstream &is) {
        std::array<char, BUFF_SIZE+1> buff;
        int bytes_read;
        do {
            bytes_read = is.readsome(buff.data(), buff.size());
            if (bytes_read <= 0) break;
            ::send(sock, buff.data(), bytes_read, 0);
#ifdef DEBUG
        std::cerr << "Send to server " << bytes_read << " bytes of file data" << std::endl;
#endif
        } while (bytes_read == BUFF_SIZE && !is.eof());
    }

    void send(int sock, std::string msg) {
        ::send(sock, msg.data(), msg.size(), 0);
#ifdef DEBUG
        std::cerr << "Sent to server: " << msg << std::endl;
#endif
    }

    static const size_t BUFF_SIZE = 1024;
};

class DataConnection : public Connection {
public:
    DataConnection(int ctrl_sock, const std::string &server_addr) {
        // flush ctrl socket to drop latecomers misc messages
        {
            std::array<char, BUFF_SIZE+1> buff;
            while (recv(ctrl_sock, buff.data(), buff.size(), MSG_DONTWAIT) == EAGAIN) {};
        }

        std::stringstream ss(ask(ctrl_sock, "PASV\n"));
        ss.ignore(std::numeric_limits<std::streamsize>::max(), '(');
        std::vector<int> buff;

        do {
            int i;
            ss >> i;
            buff.emplace_back(i);
        } while (ss.get() == ',');

        int data_port = buff[buff.size() - 2] * 256 + buff.back();
        sock = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in data_addr;
        data_addr.sin_family = AF_INET;
        data_addr.sin_port = htons(data_port);
        data_addr.sin_addr.s_addr = inet_addr(server_addr.data());

        if (connect(sock, (struct sockaddr *)&data_addr, sizeof(data_addr))) {
            std::cerr << "Data connection failed!\n";
        }
    }

    ~DataConnection() {
        close(sock);
    }

    std::string receive() {
        return Connection::receive(sock, true);
    }

    void send(std::string msg) {
        Connection::send(sock, msg);
    }

    void send(std::ifstream &is) {
        Connection::send(sock, is);
    }

private:
    int sock;
};

class FtpConnection : public Connection {

public:
    FtpConnection(std::string server_addr, std::string wd, std::string user, std::string passwd, int ctrl_port=21)
        : server_addr(server_addr), lwd(wd), rwd("") {
        ctrl_sock = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(ctrl_port);
        addr.sin_addr.s_addr = inet_addr(server_addr.data());

        (void) connect(ctrl_sock, (struct sockaddr *)&addr, sizeof(addr));

        // Greeting
        receive(ctrl_sock);

        // Authentication
        // TODO: move out from constructor
        ask(ctrl_sock, "USER " + user + "\n");
        ask(ctrl_sock, "PASS " + passwd + "\n");
    }

    ~FtpConnection() {
        close(ctrl_sock);
    }

    std::string List(std::string dirname) {
        DataConnection chan(ctrl_sock, server_addr);
        send(ctrl_sock, "LIST " + rwd + "/" + dirname + "\n");
        auto res = chan.receive();
        receive(ctrl_sock);
        // TODO: return vector of files
        return res;
    }

    void Store(std::string relpath) {
        DataConnection chan(ctrl_sock, server_addr);
        send(ctrl_sock, "STOR " + relpath + "\n");
        std::ifstream file(lwd + "/" + relpath);
        chan.send(file);
        receive(ctrl_sock);
    }

    std::string Retrieve(std::string relpath) {
        DataConnection chan(ctrl_sock, server_addr);
        ask(ctrl_sock, "RETR " + relpath + "\n");
        auto res = chan.receive();
        receive(ctrl_sock);
        return res;
    }

    void Mkdir(std::string dirname) {
        ask(ctrl_sock, "MKD " + dirname + "\n");
    }

    void Cwd(std::string dirname) {
        if (dirname == "..") {
            if (rwd == "/") return;
            int pos = rwd.rfind("/");
            rwd = rwd.substr(0, pos);
        } else if (!dirname.empty() && dirname[0] == '/') {
            rwd = dirname;
        } else {
            rwd += "/" + dirname;
        }
        ask(ctrl_sock, "CWD " + rwd + "\n");
    }

    void Status() {
        ask(ctrl_sock, "STAT\n");
    }

    std::string Pwd() {
        return rwd;
    }


private:
    int ctrl_sock;
    std::string server_addr;
    std::string lwd;
    std::string rwd;
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./ftp-client server-address work-directory" << std::endl;
        return 0;
    }

    std::string username;
    std::string passwd;
    std::cout << "Enter username to log in: ";
    std::cin >> username;
    std::cout << "Enter passwd: ";
    std::cin >> passwd;
    std::string lwd = argv[2];
    FtpConnection conn(argv[1], lwd, username, passwd);

    std::cout << conn.Pwd() << "# ";
    std::string token;
    while (std::cin >> token) {
        if (token == "rls") {
            // TODO: getline better here
            std::string dirname;
            std::cin >> dirname;
            std::cout << conn.List(dirname) << std::endl;
        } else if (token == "store") {
            std::string filename;
            std::cin >> filename;
            // TODO: refactor
            conn.Store(filename);
        } else if (token == "mkdir") {
            std::string dirname;
            std::cin >> dirname;
            conn.Store(dirname);
        } else if (token == "cat") {
            std::string filename;
            std::cin >> filename;
            std::cout << conn.Retrieve(filename) << std::endl;
        } else if (token == "rcd") {
            std::string dirname;
            std::cin >> dirname;
            conn.Cwd(dirname);
        } else if (token == "get") {
            std::string filename;
            std::cin >> filename;
            std::ofstream file(lwd + "/" + filename);
            file << conn.Retrieve(filename);
        } else {
            std::cout << "Supported commands:\n\trls <dirname>\n\tstore <filename>\n\tget <filename>\n\t"
                         "status\n\tmkdir <dirname>\n\trcd <dirname>" << std::endl;
        }

        std::cout << conn.Pwd() << "# ";
    }

    return 0;
}
