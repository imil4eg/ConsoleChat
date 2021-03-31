#pragma once

#include <memory>
#include <string>
#include <queue>

#include <boost/uuid/uuid.hpp>
#include "boost/asio/connect.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "BoostCommon.hpp"
#include "MessageRouter.hpp"

class MessageRouter;

class Connection : 
public std::enable_shared_from_this<Connection>
{
public:
    Connection(
    std::shared_ptr<MessageRouter> messageRouter,
    tcp::socket&& socket);
    
    Connection(const Connection& connection) = delete;

    void start();
    void send(std::string message);

    std::string getUserName() { return m_userName; }

    friend bool operator==(const Connection& left, const Connection& right);

private:
    void acceptAsync(const beast::error_code& ec);
    void readAsync(const beast::error_code& err, 
                   std::size_t bytes_written);
    void writeAsync(const beast::error_code& err,
                    std::size_t bytes_written);
    void requestUserNameAsync(
        const beast::error_code& err,
        std::size_t bytes_writtern);
    void onUserNameReceivedAsync(
        const beast::error_code& err,
        std::size_t bytes_writtern);

    std::shared_ptr<MessageRouter> m_messageRouter;
    websocket::stream<tcp::socket> m_webSocket;
    boost::uuids::uuid m_id;
    beast::flat_buffer m_buffer;
    std::queue<std::string> m_messages;
    std::string m_userName;
};
