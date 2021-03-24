#include <cstdlib>
#include <memory>
#include <thread>
#include <iostream>
#include <string>

#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>

#include "BoostCommon.hpp"

#include "ConnectionHandler.hpp"


void fail(beast::error_code ec, const char* what);
void doSession(tcp::socket& socket);

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cout <<
                "Usage: websocket-chat-multi <address> <port>\n" <<
                "Example:\n" <<
                "    websocket-chat-server 0.0.0.0 8080\n";
            return EXIT_FAILURE;
        }

        auto const address = net::ip::address::from_string(argv[1]);//net::ip::make_address(argv[1]); 
        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));

        ConnectionHandler connectionHandler{address, port};

        connectionHandler.run();
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}
          
void fail(beast::error_code ec, const char* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

void doSession(tcp::socket& socket)
{
    beast::error_code ec;
    beast::flat_buffer buffer;

    for (;;)
    {
        http::request<http::string_body> req;
        http::read(socket, buffer, req, ec);
        if (ec == http::error::end_of_stream)
            break;
        if (ec)
        {
            fail(ec, "read");
            return;
        }

        http::string_body::value_type body;
        body.assign("test response.");
        http::response<http::string_body> resp{
            std::piecewise_construct,
            std::make_tuple(std::move(body)),
            std::make_tuple(http::status::ok, req.version())
        };
        resp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        resp.set(http::field::content_type, "application/text");
        resp.content_length(body.size());
        resp.keep_alive(req.keep_alive());

        http::serializer<false, http::string_body> sr(resp);
        http:write(socket, sr, ec);
    }
}