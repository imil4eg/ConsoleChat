#include <thread>
#include <iostream>
#include <signal.h>

#include "MessageExchanger.hpp"

void sigint_handler(int s)
{
    exit(EXIT_SUCCESS);
}

MessageExchanger::MessageExchanger(net::io_context& ioc) : 
    m_webSocket{ioc},
    m_resolver{ioc}
{
    signal(SIGINT, sigint_handler);
}

MessageExchanger::~MessageExchanger()
{
    std::cout << "Closing connection.\n";
    m_webSocket.close(websocket::close_code::normal);
}

void MessageExchanger::run(std::string host, std::string port)
{
    auto const results{m_resolver.resolve(host, port)};

    net::connect(m_webSocket.next_layer(), results.begin(), results.end());

    m_webSocket.set_option(websocket::stream_base::decorator(
        [](auto& req)
        {
            req.set(http::field::user_agent,
            std::string(BOOST_BEAST_VERSION_STRING) + 
            " websocket-client-coro");
        }
    ));

    m_webSocket.handshake(host, "/");

    std::thread requestThread(&MessageExchanger::listenRequests, this);
    std::thread consoleThread(&MessageExchanger::listenConsoleInput, this);

    requestThread.join();
    consoleThread.join();
}

void MessageExchanger::listenRequests()
{
    beast::flat_buffer buffer;
    for (;;)
    {
        m_webSocket.read(buffer);

        std::cout << beast::make_printable(buffer.data()) << '\n';
    
        buffer.clear();
    }
}

void MessageExchanger::listenConsoleInput()
{
    std::string message;
    while (std::getline(std::cin, message))
    {
        m_webSocket.write(net::buffer(message));
    }
}