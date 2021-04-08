#pragma once

#include <unordered_map>
#include <memory>

#include <boost/uuid/uuid.hpp>
#include <boost/container_hash/hash.hpp>

#include "BoostCommon.hpp"
#include "Connection.hpp"
#include "Room.hpp"

class Connection;
class Room;

class RoomsHolder
{
public:
    RoomsHolder() : 
        m_rooms{}
    {   
    }

    void insert(std::shared_ptr<Room> room);
    bool tryRemove(const boost::uuids::uuid& id);
    bool tryJoinRoom(const boost::uuids::uuid& id, 
                  std::shared_ptr<Connection> connection);

private:
    std::unordered_map<boost::uuids::uuid, std::shared_ptr<Room>, boost::hash<boost::uuids::uuid>> m_rooms;
};