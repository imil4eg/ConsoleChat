#include <iostream>

#include <boost/uuid/uuid_generators.hpp>

#include "Connection.hpp"

#define UNUSED(x) (void)(x)

Connection::Connection(
    std::shared_ptr<MessageRouter> messageRouter,
    tcp::socket&& socket) : 
    m_messageRouter{messageRouter},
    m_webSocket{std::move(socket)},
    m_id{boost::uuids::random_generator()()},
    m_messages{}
{    
}

void Connection::start()
{   
    try
    {
        m_webSocket.set_option(websocket::stream_base::decorator(
            [](websocket::response_type& res)
            {
                res.set(http::field::server,
                std::string(BOOST_BEAST_VERSION_STRING) + 
                " websocket-server-sync");
            }
        ));

        m_webSocket.async_accept(
            beast::bind_front_handler(
                &Connection::acceptAsync,
             shared_from_this()));
    }
    catch(const beast::system_error& se)
    {
        if (se.code() != websocket::error::closed)
            std::cerr << "Error: " << se.code().message() << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
}
 
void Connection::send(std::string message)
{
    m_messages.push(message);

    if (m_messages.size() > 1)
        return;
    
    m_webSocket.async_write(
        net::buffer(message),
        beast::bind_front_handler(
            &Connection::writeAsync,
        shared_from_this()));
}

void Connection::acceptAsync(const beast::error_code& ec)
{
    if (ec)
    {
        m_messageRouter->disconnect(*this);
        return;
    }

    m_webSocket.async_read(m_buffer,
    beast::bind_front_handler(
        &Connection::readAsync,
    shared_from_this()));
}

void Connection::readAsync(const beast::error_code& err, 
                   std::size_t bytes_written)
{
    UNUSED(bytes_written);
    std::cout << "Accepted message\n";

    if (err == http::error::end_of_stream || 
        err == net::error::eof)
    {
        std::cout << "User disconnected.\n";
        m_messageRouter->disconnect(*this);
        return;
    }

    if (err)
    {
        std::cerr << "read: " << err.message() << '\n';
        return;  
    }

    m_messageRouter->send(beast::buffers_to_string(m_buffer.data()), shared_from_this());

    m_buffer.clear();

    m_webSocket.async_read(m_buffer,
    beast::bind_front_handler(
        &Connection::readAsync,
        shared_from_this()
    ));
}

void Connection::writeAsync(const beast::error_code& err,
                    std::size_t bytes_written)
{
    UNUSED(bytes_written);

    if (err)
    {
        std::cerr << "write: " << err.message() << '\n';
        return;
    }

    std::cout << m_messages.front() << " message was sent.\n";

    m_messages.pop();

    if (!m_messages.empty())
    {
        m_webSocket.async_write(
            net::buffer(m_messages.front()),
            beast::bind_front_handler(
                &Connection::writeAsync,
                shared_from_this()
            )
        );   
    }
}

bool operator==(const Connection& left, const Connection& right)
{
    return left.m_id == right.m_id;
}