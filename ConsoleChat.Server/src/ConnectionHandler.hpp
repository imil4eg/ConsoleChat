#pragma once

#include <vector>

#include <boost/enable_shared_from_this.hpp>

#include "BoostCommon.hpp"
#include "MessageRouter.hpp"

class ConnectionHandler : 
public std::enable_shared_from_this<ConnectionHandler>
{
public:
    ConnectionHandler(net::io_context& ioc,
                      const tcp::endpoint endpoint);
    ~ConnectionHandler();

    void run();

private:
    void asyncAccept(const boost::system::error_code& error,
                     tcp::socket socket);

    net::io_context& m_context;
    tcp::acceptor m_acceptor;
    std::shared_ptr<MessageRouter> m_messageRouter;
};