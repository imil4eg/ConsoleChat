#pragma once

#include <vector>
#include <string>
#include <memory>

#include "Connection.hpp"

class Connection;

class MessageRouter
{
public:
    MessageRouter() : 
        m_connections{}
    {    
    }

    void join(std::unique_ptr<Connection> connection);
    void send(std::string message);
    void disconnect(Connection& connection);
    
private:
    std::vector<std::unique_ptr<Connection>> m_connections;
};