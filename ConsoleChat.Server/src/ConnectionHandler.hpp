#pragma once

#include <vector>

#include "BoostCommon.hpp"
#include "MessageRouter.hpp"

class ConnectionHandler
{
public:
    ConnectionHandler(const net::ip::address& address, unsigned short port);
    ~ConnectionHandler();

    void run();

private:
    MessageRouter m_messageRouter;
    net::io_context m_context;
    tcp::acceptor m_acceptor;
};