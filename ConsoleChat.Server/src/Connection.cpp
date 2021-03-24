#include <iostream>

#include <boost/uuid/uuid_generators.hpp>

#include "Connection.hpp"

Connection::Connection(
    MessageRouter& messageRouter,
    tcp::socket&& socket) : 
    m_messageRouter{&messageRouter},
    m_webSocket{std::move(socket)},
    m_id{boost::uuids::random_generator()()}
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

        m_webSocket.accept();

        beast::error_code err;
        beast::flat_buffer buffer;
        for (;;)
        {
            m_webSocket.read(buffer, err);

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

            m_messageRouter->send(beast::buffers_to_string(buffer.data()));

            buffer.clear();
        }
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
    std::cout << "Sending message " << message << '\n';
    m_webSocket.write(net::buffer(message));
}

bool operator==(const Connection& left, const Connection& right)
{
    return left.m_id == right.m_id;
}