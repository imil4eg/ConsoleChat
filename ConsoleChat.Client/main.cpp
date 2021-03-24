#include <iostream>

#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>
#include <boost/asio/connect.hpp>

#include "BoostCommon.hpp"
#include "MessageExchanger.hpp"

int main(int argc, char* argv[])
{
    try
    {
        if (argc < 3)
        {
            std::cout <<
                "Usage: websocket-chat-multi <address> <port>\n" <<
                "Example:\n" <<
                "    websocket-chat-server 0.0.0.0 8080\n";
            return EXIT_FAILURE;
        }

        auto const serverHost{argv[1]};
        auto const serverPort{argv[2]};

        net::io_context ioc{1};
        
        MessageExchanger exchanger(ioc);
        exchanger.run(serverHost, serverPort);

        std::cout << "Closing connection\n";

        ioc.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}