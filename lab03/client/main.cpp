#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

using namespace boost::beast;
namespace net = boost::asio;

#include <iostream>
#include <string>
#include <thread>
#include <fstream>

const size_t BUFF_SIZE = 1024;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cout << "Usage: ./client server_host server_port filename\n";
        return 0;
    }

    std::string server_address(argv[1]);
    const uint16_t port = std::atoi(argv[2]);
    std::string filename(argv[3]);

    // Connect
    net::io_context context;
    tcp_stream stream(context);
    stream.connect({net::ip::make_address(server_address), port});

    // Prepare and send request
    http::request<http::string_body> req{http::verb::get, "/" + filename, 11};
    req.set(http::field::host, server_address);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    http::write(stream, req);

    // Get response
    flat_buffer buf;
    http::response<http::dynamic_body> res;
    http::read(stream, buf, res);

    std::cout << "Server response:\n" << res << std::endl;

    {
        error_code ec;
        stream.socket().shutdown(net::ip::tcp::socket::shutdown_both, ec);
        if (ec.failed()) {
            std::cerr << "Socket closed abnormally: " << ec.message() << '\n';
            return 1;
        }
    }

    return 0;
}
