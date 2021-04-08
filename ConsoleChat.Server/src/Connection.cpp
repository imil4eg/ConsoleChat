#include <iostream>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "Connection.hpp"
#include "Logger.hpp"

#define UNUSED(x) (void)(x)

Connection::Connection(
    tcp::socket&& socket,
    std::shared_ptr<RoomsHolder> roomsHolder) : 
    m_webSocket{std::move(socket)},
    m_id{boost::uuids::random_generator()()},
    m_messages{},
    m_userName{},
    m_roomsHolder{roomsHolder},
    m_room{nullptr}
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

void Connection::askActionOnRoomRemove()
{
    m_webSocket.async_write(
        net::buffer("Room was removed.\n" + 
        m_commandMessage),
        beast::bind_front_handler(
            &Connection::onCommandReadAsync,
            shared_from_this()
        )
    );
}

void Connection::acceptAsync(const beast::error_code& ec)
{
    if (ec)
    {
        logger::fail(ec, "accept");
        return;
    }

    m_webSocket.async_write(
        net::buffer("Please, enter user name:"),
        beast::bind_front_handler(
            &Connection::requestUserNameAsync,
            shared_from_this()
        )
    );
}

void Connection::readAsync(const beast::error_code& err, 
                   std::size_t bytes_written)
{
    UNUSED(bytes_written);
    std::cout << "Accepted message\n";

    if (err == http::error::end_of_stream || 
        err == net::error::eof)
    {
        std::cout << m_userName << " disconnected.\n";
        m_room->disconnect(shared_from_this());
        return;
    }

    if (err)
    {
        std::cerr << "read: " << err.message() << '\n';
        return;  
    }

    std::string message{beast::buffers_to_string(m_buffer.data())};

    if (boost::iequals(message, m_exitRoomCommandMessage))
    {
        exitRoom();
        return;
    }

    m_room->sendMessage(beast::buffers_to_string(m_buffer.data()), shared_from_this());

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

void Connection::requestUserNameAsync(
    const beast::error_code& err,
        std::size_t bytes_writtern)
{   
    UNUSED(bytes_writtern);

    if (err)
    {
        std::cout << "write: " << err.message() << '\n';
        return;
    }

    m_webSocket.async_read(
        m_buffer,
        beast::bind_front_handler(
            &Connection::onUserNameReceivedAsync,
            shared_from_this()
        )
    );
}

void Connection::onUserNameReceivedAsync(
    const beast::error_code& err,
        std::size_t bytes_writtern)
{
    UNUSED(bytes_writtern);

    if (err)
    {
        std::cout << "error: " << err.message() << '\n';
        return;
    }    

    m_userName = beast::buffers_to_string(m_buffer.data());

    m_buffer.clear();

    std::cout << m_userName << " joined the server\n";

    m_webSocket.async_write(
        net::buffer(m_commandMessage),
        beast::bind_front_handler(
            &Connection::requestCommandWriteAsync,
            shared_from_this()
        )
    );
}

void Connection::requestCommandWriteAsync(const beast::error_code& err,
        std::size_t bytes_writtern)
{
    UNUSED(bytes_writtern);

    if (err)
    {
        logger::fail(err, "write");
        return;
    }

    m_webSocket.async_read(
        m_buffer,
        beast::bind_front_handler(
            &Connection::onCommandReadAsync,
            shared_from_this()
        )
    );
}

void Connection::onCommandReadAsync(
        const beast::error_code& err,
        std::size_t bytes_writtern)
{
    UNUSED(bytes_writtern);

    if (err)
    {
        logger::fail(err, "read");
        return;
    }

    std::string command{beast::buffers_to_string(m_buffer.data())};

    if (command.empty())
    {
        sendUnknownCommandMessage();
        return;
    }

    auto params{splitParams(command)};
    if (boost::iequals(params[0], m_createRoomCommandMessage) ||
        command == "1")
    {
        createRoom(params);
    }
    else if (boost::iequals(params[0], m_joinRoomCommandMessage) || 
             command == "2")
    {
        joinRoom(params);
    }
    else if (boost::iequals(params[0], m_deleteRoomCommandMessage) || 
             command == "3")
    {
        deleteRoom(params);
    }
    else
    {
        sendUnknownCommandMessage();
    }

    m_buffer.clear();
}

void Connection::sendUnknownCommandMessage()
{
    m_webSocket.async_write(
            net::buffer("Unknown command.\n" +
             m_commandMessage),
             beast::bind_front_handler(
                 &Connection::requestCommandWriteAsync,
                 shared_from_this()
             )
        );
}

void Connection::onRoomCreatedAsync(const beast::error_code& err,
        std::size_t bytes_written)
{
    UNUSED(bytes_written);

    if (err)
    {
        logger::fail(err, "write");
        return;
    }

    m_webSocket.async_read(
        m_buffer,
        beast::bind_front_handler(
            &Connection::readAsync,
            shared_from_this()
        )
    );
}

void Connection::sendIncorrectNumberOfArgumentsMessage(const std::string& commandName,
        int requiredParamsCount)
{
    m_webSocket.async_write(
        net::buffer("Command " + commandName + " requires " + 
        std::to_string(requiredParamsCount) + 
        " parameters. Please, try again.\n" + m_commandMessage),
        beast::bind_front_handler(
            &Connection::requestCommandWriteAsync,
            shared_from_this()
        )
    );    
}

void Connection::createRoom(const std::vector<std::string>& params)
{
    if (params.size() != 2)
    {
        sendIncorrectNumberOfArgumentsMessage(m_createRoomCommandMessage, 1);
    }
    else
    {
        std::string roomName{params[1]};
        auto createdRoom{std::make_shared<Room>(roomName)};

        m_room = createdRoom;
        m_room->join(shared_from_this());
        m_roomsHolder->insert(createdRoom);

        std::cout << "Created the room with name " 
                  << roomName << '\n';

        m_webSocket.async_write(
            net::buffer("Create room with name " + roomName + 
                        ". Room ID to join: " + to_string(m_room->getId()) + 
                        "\nEnter " + m_exitRoomCommandMessage + " command to exit.\n"),
            beast::bind_front_handler(
                &Connection::onRoomCreatedAsync,
                shared_from_this()
            )
        );
    }
}

void Connection::joinRoom(const std::vector<std::string>& params)
{
    if (params.size() < 2)
    {
        sendIncorrectNumberOfArgumentsMessage(m_joinRoomCommandMessage, 1);
        return;
    }

    Room::id_t roomId{boost::lexical_cast<Room::id_t>(params[1])};
    
    if (m_roomsHolder->tryJoinRoom(roomId, shared_from_this()))
    {
        m_webSocket.async_write(
            net::buffer("Successfully joined " + m_room->getName() +
             " room.\nEnter " + m_exitRoomCommandMessage + " to exit.\n"),
            beast::bind_front_handler(
                &Connection::onWriteAsync,
                shared_from_this()
            )
        );
    }
    else
    {
        m_webSocket.async_write(
            net::buffer("Couldn't join the room with id " + params[1]
            + ". Please, try again.\n"),
            beast::bind_front_handler(
                &Connection::requestCommandWriteAsync,
                shared_from_this()
            )
        );
    }                
}

void Connection::onWriteAsync(const beast::error_code& err,
        std::size_t bytes_written)
{
    UNUSED(bytes_written);

    if (err)
    {
        logger::fail(err, "write");
        return;
    }    

    m_webSocket.async_read(
        m_buffer,
        beast::bind_front_handler(
            &Connection::readAsync,
            shared_from_this()
        )
    );
}

void Connection::deleteRoom(const std::vector<std::string>& params)
{
    if (params.size() < 2)
    {
        sendIncorrectNumberOfArgumentsMessage(m_deleteRoomCommandMessage, 1);
        return;
    }
    
    Room::id_t id{boost::lexical_cast<Room::id_t>(params[1])};

    if (m_roomsHolder->tryRemove(id))
    {
        std::cout << "Removed room with id " << id;
        m_webSocket.async_write(
            net::buffer("Room with "  + params[1] + " id was " + 
             "succesfully removed.\n"),
             beast::bind_front_handler(
                 &Connection::onWriteAsync,
                 shared_from_this()
             )
        );
    }
    else
    {
        m_webSocket.async_write(
            net::buffer("Couldn't remove room with id " + params[1] + 
            " due to some users still in it.\n"),
            beast::bind_front_handler(
                &Connection::requestCommandWriteAsync,
                shared_from_this()
            )
        );
    }
}

std::vector<std::string> Connection::splitParams(const std::string command)
{
    std::vector<std::string> params;
    std::string::size_type current = 0;
    std::string::size_type prev = 0;
    while(current < command.size())
    {
        if (command[current] == ' ')
        {
            params.push_back(command.substr(prev, current - prev));
            prev = current + 1;
        }

        ++current;
    }

    params.push_back(command.substr(prev, current - prev));

    return params;
}

void Connection::exitRoom()
{
    std::cout << m_userName << " exit room " << m_room->getName() << '\n';

    m_webSocket.async_write(
        net::buffer("Left room with name " + m_room->getName() + '\n' + 
        m_commandMessage),
        beast::bind_front_handler(
            &Connection::requestCommandWriteAsync,
            shared_from_this()
        )
    );

    m_room->disconnect(shared_from_this());
    m_room = nullptr;
}

bool operator==(const Connection& left, const Connection& right)
{
    return left.m_id == right.m_id;
}