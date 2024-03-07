#include <boost/beast/http.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/core/file_base.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/asio/ip/tcp.hpp>

using namespace boost::beast;
namespace net = boost::asio;

#include <iostream>
#include <string>
#include <thread>
#include <fstream>

const size_t BUFF_SIZE = 1024;
const std::string server_address = "127.0.0.1";
const std::string directory_to_share = "shared";

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: ./server server_port\n";
        return 0;
    }

    net::io_context context{1};
    const uint16_t port = std::atoi(argv[1]);
    net::ip::tcp::acceptor listener(context, {net::ip::make_address(server_address), port});
    std::cerr << "Server started\n";

    while (1) {

        net::ip::tcp::socket sock{context};
        listener.accept(sock);

        auto session = [] (auto &&sock) -> void {
            // Get request
            http::request<http::string_body> req;
            {
                error_code ec;
                flat_buffer buff;
                http::read(sock, buff, req, ec);
                if (ec.failed()) {
                    std::cerr << "Read failed: " << ec.message() << "\n";
                    return;
                }
            }

            std::cerr << "Request:\n" << req.base().target() << "\n";
            std::string requested_filename = directory_to_share + static_cast<std::string>(req.base().target());

            // Prepare response
            http::file_body::value_type body;
            {
                error_code ec;
                body.open(requested_filename.data(), file_mode::read, ec);
                if (ec.failed()) {
                    std::string msg = "Failed to open " + requested_filename + ": " + ec.message();
                    http::response<http::string_body> res{http::status::not_found, req.version()};
                    res.set(http::field::content_type, "text/html");
                    res.keep_alive(false);
                    res.body() = msg;
                    res.prepare_payload();
                    std::cerr << msg << std::endl;
                    write(sock, res, ec);
                } else {
                    std::cerr << "Successfully opened " << requested_filename << '\n';
                    http::response<http::file_body> res{
                        std::piecewise_construct,
                        std::make_tuple(std::move(body)),
                        std::make_tuple(http::status::ok, req.version())};
                    res.keep_alive(false);
                    res.set(http::field::content_type, "text/plain");
                    write(sock, res, ec);
                    if (ec.failed()) {
                        std::cerr << "Failed to send response: " << ec.message() << '\n';
                        return;
                    }
                }
            }

            // Close socket
            {
                error_code ec;
                sock.shutdown(net::ip::tcp::socket::shutdown_send, ec);
                if (ec.failed()) {
                    std::cerr << "Socket closed abnormally: " << ec.message() << '\n';
                    return;
                }
            }
            std::cerr << "Connection closed\n";
        };

        std::thread { std::bind(session, std::move(sock)) }.detach();
    }

    std::cerr << "Server terminated\n";

    return 0;
}
