#include "room.h"

Room::Room(const QString& roomName,
           quint16 port,
           const QString& hostUsername,
           const GameSettings& settings,
           const QString& password)
    : roomName(roomName)
    , port(port)
    , hostUsername(hostUsername)
    , password(password)
    , gameSettings(settings)
{
}