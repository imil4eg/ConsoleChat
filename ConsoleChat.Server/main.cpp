#include <cstdlib>
#include <memory>
#include <thread>
#include <iostream>
#include <string>

#include <boost/beast/version.hpp>
#include <boost/beast/core.hpp>

#include "BoostCommon.hpp"

#include "ConnectionHandler.hpp"

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 4)
        {
            std::cout <<
                "Usage: websocket-chat-multi <address> <port> <threads>\n" <<
                "Example:\n" <<
                "    websocket-chat-server 0.0.0.0 8080 1\n";
            return EXIT_FAILURE;
        }

        auto const address = net::ip::address::from_string(argv[1]); 
        auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
        int threadsCount = std::max(1, std::atoi(argv[3]));

        net::io_context ioc{threadsCount};

        std::make_shared<ConnectionHandler>(ioc, tcp::endpoint(address, port))->run();

        std::vector<std::thread> threads(threadsCount);
        for (int i = threadsCount - 1; i > 0; --i)
        {
            threads.emplace_back(
                [&ioc]
            {
                ioc.run();
            });
        }

        ioc.run();

        for (auto& t : threads)
            t.join();
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << '\n';
    }
}