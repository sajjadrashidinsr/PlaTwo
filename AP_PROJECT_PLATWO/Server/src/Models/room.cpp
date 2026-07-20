#include "room.h"
#include "../Game/GameController.h"

Room::Room(const QString& roomName,
           quint16 port,
           const QString& hostUsername,
           QTcpSocket* hostSocket,
           const GameSettings& settings,
           const QString& password)
    : roomName(roomName)
    , port(port)
    , hostUsername(hostUsername)
    , hostSocket(hostSocket)
    , password(password)
    , gameSettings(settings)
{
}

