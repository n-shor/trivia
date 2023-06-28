#include "RoomManager.h"
#include <iostream>

std::map<unsigned int, Room> RoomManager::m_rooms;
int RoomManager::currId = 0;
std::mutex RoomManager::m_roomsMutex;


void RoomManager::createRoom(RoomData& rd, LoggedUser lu)
{
    std::lock_guard<std::mutex> lock(m_roomsMutex);
    m_rooms[rd.id] = Room(rd);
    m_rooms[rd.id].addUser(lu);
}

void RoomManager::deleteRoom(int ID)
{
    std::lock_guard<std::mutex> lock(m_roomsMutex);
    for (auto it = m_rooms.begin(); it != m_rooms.end();)
    {
        if (it->first == ID)
        {
            m_rooms.erase(it);
            return;
        }
        else
        {
            ++it;
        }
    }
}

const unsigned int RoomManager::getRoomState(int ID) const
{
    return m_rooms[ID].getRoomData().isActive;
}

const std::vector<RoomData> RoomManager::getRooms() const
{
    std::lock_guard<std::mutex> lock(m_roomsMutex);

    std::vector<RoomData> ret;
    for (auto& roomPair : m_rooms) {
        ret.push_back(roomPair.second.getRoomData());
    }
    return ret;
}


Room& RoomManager::getRoom(int ID) const
{
    return m_rooms[ID];
}
