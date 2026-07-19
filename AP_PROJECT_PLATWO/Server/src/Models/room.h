#ifndef ROOM_H
#define ROOM_H

#include <QString>
#include "GameSettings.h"
#include <QTcpSocket>
#include "GameController.h"

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

    std::unique_ptr<GameController> gameController;
    bool gameStarted = false;
    QString gameType = "DotsAndBoxes";
    int currentPlayerId = 0;
};

#endif // ROOM_H