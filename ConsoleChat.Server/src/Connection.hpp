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
#include "Room.hpp"
#include "RoomsHolder.hpp"

class Room;
class RoomsHolder;

class Connection : 
public std::enable_shared_from_this<Connection>
{
public:
    Connection(
    tcp::socket&& socket,
    std::shared_ptr<RoomsHolder> roomsHolder);
    
    Connection(const Connection& connection) = delete;

    void start();
    void send(std::string message);
    void askActionOnRoomRemove();

    std::string getUserName() const { return m_userName; }
    void setRoom(std::shared_ptr<Room> room) { m_room = room; }

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
    void requestCommandWriteAsync(
        const beast::error_code& err,
        std::size_t bytes_writtern);
    void onCommandReadAsync(
        const beast::error_code& err,
        std::size_t bytes_writtern);
    void onRoomCreatedAsync(const beast::error_code& err,
        std::size_t bytes_written);
    void sendIncorrectNumberOfArgumentsMessage(
        const std::string& commandName,
        int requiredParamsCount);
    void onWriteAsync(const beast::error_code& err,
        std::size_t bytes_written);
    

    void createRoom(const std::vector<std::string>& params);
    void joinRoom(const std::vector<std::string>& params);
    void deleteRoom(const std::vector<std::string>& params);
    void sendUnknownCommandMessage();
    void exitRoom();

    std::vector<std::string> splitParams(const std::string command);

    websocket::stream<tcp::socket> m_webSocket;
    boost::uuids::uuid m_id;
    beast::flat_buffer m_buffer;
    std::queue<std::string> m_messages;
    std::string m_userName;
    std::shared_ptr<RoomsHolder> m_roomsHolder;
    std::shared_ptr<Room> m_room;

    const std::string m_createRoomCommandMessage{
        "CreateRoom"
    };
    const std::string m_createRoomCommandParams{
        "<roomName>"
    };
    const std::string m_joinRoomCommandMessage{
        "JoinRoom"
    };
    const std::string m_joinRoomCommandParams{
        "<roomId>"
    };
    const std::string m_deleteRoomCommandMessage{
        "DeleteRoom"
    };
    const std::string m_deleteRoomCommandParams{
        "<roomId>"
    };
    const std::string m_exitRoomCommandMessage
    {
        "ExitRoom"
    };
    std::string m_commandMessage{"Please, enter the command:\n"
        "1) " + m_createRoomCommandMessage + " " + m_createRoomCommandParams + 
        "\n2) " + m_joinRoomCommandMessage + " " + m_joinRoomCommandParams +  
        "\n3) " + m_deleteRoomCommandMessage + " " + m_deleteRoomCommandParams +  
        '\n'
    };    
};
