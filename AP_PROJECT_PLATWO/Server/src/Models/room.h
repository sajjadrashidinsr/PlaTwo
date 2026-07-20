#ifndef ROOM_H
#define ROOM_H

#include <QString>
#include <QTcpSocket>
#include <memory>
#include "GameSettings.h"

class GameController;

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

    ~Room() = default;

    Room(Room&& other) noexcept = default;
    Room& operator=(Room&& other) noexcept = default;

    Room(const Room&) = delete;
    Room& operator=(const Room&) = delete;

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

    std::shared_ptr<GameController> gameController;

    bool gameStarted = false;
    QString gameType = "DotsAndBoxes";
    int currentPlayerId = 0;
};

#endif // ROOM_H