#include "MessageRouter.hpp"

void MessageRouter::join(std::unique_ptr<Connection> connection)
{
    m_connections.push_back(std::move(connection));
}

void MessageRouter::send(std::string message, const Connection& sender)
{
    for (auto& connection : m_connections)
    {
        if (*connection == sender)
            continue;

        connection->send(message);
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