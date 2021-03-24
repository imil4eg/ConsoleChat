#pragma once

#include <memory>
#include <string>

#include <boost/uuid/uuid.hpp>
#include "boost/asio/connect.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include "BoostCommon.hpp"
#include "MessageRouter.hpp"

class MessageRouter;

class Connection
{
public:
    Connection(
    MessageRouter& messageRouter,
    tcp::socket&& socket);
    
    Connection(const Connection& connection) = delete;

    void start();
    void send(std::string message);

    friend bool operator==(const Connection& left, const Connection& right);

private:
    MessageRouter* m_messageRouter;
    websocket::stream<tcp::socket> m_webSocket;
    boost::uuids::uuid m_id;
};
