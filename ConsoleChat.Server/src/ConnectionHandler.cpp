#include <iostream>
#include <boost/thread.hpp>

#include "ConnectionHandler.hpp"
#include "Connection.hpp"

ConnectionHandler::ConnectionHandler(
    const net::ip::address& address,
    unsigned short port) :
    m_context{1},
    m_acceptor{m_context, {address, port}},
    m_messageRouter{}
{
}

ConnectionHandler::~ConnectionHandler() = default;

void ConnectionHandler::run()
{
    std::cout << "Server started.\n";

    for (;;)
    {
        tcp::socket socket{m_context};
        m_acceptor.accept(socket);

        std::cout << "Received connection\n";

        auto connection{
            std::make_unique<Connection>(m_messageRouter, 
                                         std::move(socket))
        };

        boost::thread(boost::bind(&Connection::start, &*connection));
        
        m_messageRouter.join(std::move(connection));
    }
}