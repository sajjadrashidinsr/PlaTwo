#ifndef ROOM_H
#define ROOM_H

#include <QString>
#include "GameSettings.h"
#include <QTcpSocket>

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
         QTcpSocket* hostSocket,
         const GameSettings& settings,
         const QString& password = QString());

    QString roomName;
    quint16 port = 1234;
    QString hostUsername;
    QTcpSocket* hostSocket = nullptr;
    QString guestUsername;
    QTcpSocket* guestSocket = nullptr;
    QString password;
    GameSettings gameSettings;
    RoomStatus status = RoomStatus::Waiting;

    bool hasGuest() const { return !guestUsername.isEmpty(); }
    bool isPasswordProtected() const { return !password.isEmpty(); }
};

#endif // ROOM_H