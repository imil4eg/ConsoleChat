#include <boost/uuid/random_generator.hpp>

#include "Room.hpp"

Room::Room(const std::string& name) : 
    m_name{name},
    m_id{boost::uuids::random_generator()()}
{
}

void Room::join(std::shared_ptr<Connection> connection)
{
    m_connections.push_back(connection);

    sendMessage(connection->getUserName() + " joined the room.\n", connection);
}

void Room::sendMessage(std::string message, std::shared_ptr<Connection> sender)
{
    for (auto connection : m_connections)
    {
        if (*connection == *sender)
            continue;

        connection->send(sender->getUserName() + ": " + message);
    }
}

void Room::disconnect(std::shared_ptr<Connection> connection)
{
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
    {
        if (*it == connection)
        {
            m_connections.erase(it);
            sendMessage(connection->getUserName() + " has left the room.\n", connection);
            return;
        }
    }
}