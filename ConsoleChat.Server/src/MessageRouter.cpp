#include "MessageRouter.hpp"

void MessageRouter::join(std::shared_ptr<Connection> connection)
{
    m_connections.push_back(connection);
}

void MessageRouter::send(std::string message, std::shared_ptr<Connection> sender)
{
    for (auto connection : m_connections)
    {
        if (*connection == *sender)
            continue;

        connection->send(sender->getUserName() + ": " + message);
    }
}

void MessageRouter::disconnect(Connection& connection)
{
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it)
    {
        if (**it == connection)
        {
            m_connections.erase(it);
            return;
        }
    }
}