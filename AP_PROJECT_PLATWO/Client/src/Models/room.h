#ifndef ROOM_H
#define ROOM_H

#include <QString>
#include "GameSettings.h"

enum class RoomStatus
{
    Waiting,
    InProgress,
    Finished
};

class Room
{
public:
    Room() = default;
    Room(const QString& roomName,
         quint16 port,
         const QString& hostUsername,
         const GameSettings& settings,
         const QString& password = QString());

    QString roomName;
    quint16 port = 1234;
    QString hostUsername;
    QString guestUsername;
    QString password;
    GameSettings gameSettings;
    RoomStatus status = RoomStatus::Waiting;

    bool hasGuest() const { return !guestUsername.isEmpty(); }
    bool isPasswordProtected() const { return !password.isEmpty(); }
};

#endif // ROOM_H
