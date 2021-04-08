#include <iostream>

#include "ConnectionHandler.hpp"
#include "Connection.hpp"

ConnectionHandler::ConnectionHandler(net::io_context& ioc,
    const tcp::endpoint endpoint) :
    m_context{ioc},
    m_acceptor{ioc},
    m_roomsHolder{std::make_shared<RoomsHolder>()}
{
    beast::error_code ec;

    m_acceptor.open(endpoint.protocol(), ec);
    if (ec)
    {
        std::cout << "Failed to open\n";
        return;
    }

    m_acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if (ec)
    {
        std::cout << "Failed to set option\n";
        return;
    }

    m_acceptor.bind(endpoint, ec);
    if (ec)
    {
        std::cout << "Failed to bind\n";
        return;
    }

    m_acceptor.listen(net::socket_base::max_listen_connections, ec);
    if (ec)
    {
        std::cout << "Failed to listen\n";
        return;
    }
}

ConnectionHandler::~ConnectionHandler() = default;

void ConnectionHandler::run()
{
    std::cout << "Server started.\n";

    m_acceptor.async_accept(
        beast::bind_front_handler(
            &ConnectionHandler::asyncAccept, 
            shared_from_this()));
}

void ConnectionHandler::asyncAccept(const boost::system::error_code& error,
                     tcp::socket socket)
{
    std::cout << "Received connection\n";

    if (error)
    {
        std::cout << "Error: " << error.message() << '\n';
        return;
    }

    auto connection{
        std::make_shared<Connection>(std::move(socket), m_roomsHolder)
    };

    connection->start();

    m_acceptor.async_accept(
        beast::bind_front_handler(
            &ConnectionHandler::asyncAccept,
            shared_from_this()
        )
    );
}