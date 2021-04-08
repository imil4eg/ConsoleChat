#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "RoomsHolder.hpp"

void RoomsHolder::insert(std::shared_ptr<Room> room)
{
    m_rooms.insert({room->getId(), room});
}

bool RoomsHolder::tryRemove(const Room::id_t& id)
{
    if (m_rooms[id]->getConnections().size() > 1                                                                                                                                                                                                                                                                                                                                                                                                                        )
        return false;

    for (auto& connection : m_rooms[id]->getConnections())
        connection->askActionOnRoomRemove();

    m_rooms.erase(id);

    return true;
}

bool RoomsHolder::tryJoinRoom(const Room::id_t& id, std::shared_ptr<Connection> connection)
{
    if (m_rooms.find(id) == m_rooms.end())
        return false;

    m_rooms[id]->join(connection);
    connection->setRoom(m_rooms[id]);
    
    return true;
}

