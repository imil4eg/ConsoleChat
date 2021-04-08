#pragma once

#include <vector>
#include <memory>
#include <string>

#include <boost/uuid/uuid.hpp>

#include "Connection.hpp"

class Connection;

class Room
{
public:
    typedef boost::uuids::uuid id_t;

    Room(const std::string& name);

    void join(std::shared_ptr<Connection> connection);
    void sendMessage(std::string message, std::shared_ptr<Connection> sender);
    void disconnect(std::shared_ptr<Connection> connection);
    
    const std::vector<std::shared_ptr<Connection>>& getConnections() const
    {
        return m_connections;
    }
    const std::string& getName() const { return m_name; }

    const id_t& getId() const { return m_id; }

private:
    std::vector<std::shared_ptr<Connection>> m_connections;
    std::string m_name;
    id_t m_id;
};