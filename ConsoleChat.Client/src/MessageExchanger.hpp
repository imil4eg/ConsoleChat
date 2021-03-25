#pragma once

#include <string>

#include "BoostCommon.hpp"

class MessageExchanger
{
public:
    MessageExchanger(net::io_context& ioc);
    ~MessageExchanger();

    void run(std::string host, std::string port);

private:
    void listenRequests();
    void listenConsoleInput();
    
    websocket::stream<tcp::socket> m_webSocket;
    tcp::resolver m_resolver;
};