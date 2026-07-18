#include "roommanager.h"
#include "network_constants.h"
#include "network_protocol.h"
#include "qjsonobject.h"
#include <QDebug>

RoomManager::RoomManager(QObject *parent)
    : QObject(parent)
{
}

bool RoomManager::createRoom(const QString& roomName, quint16 port,
                             const QString& hostUsername,
                             QTcpSocket* hostSocket,
                             const GameSettings& settings,
                             const QString& password,
                             Room* outRoom)
{
    QMutexLocker locker(&mutex);

    if (rooms.contains(port)) {
        qDebug() << "[RoomManager] Port" << port << "already in use.";
        return false;
    }

    Room room(roomName, port, hostUsername, hostSocket, settings, password);
    rooms[port] = room;

    qDebug() << "[RoomManager] Room created:" << roomName << "port:" << port << "host:" << hostUsername;

    if (outRoom) {
        *outRoom = room;
    }

    emit roomCreated(port);
    return true;
}

bool RoomManager::joinRoom(quint16 port, const QString& guestUsername,
                           QTcpSocket* guestSocket,
                           const QString& password, Room* outRoom)
{
    QMutexLocker locker(&mutex);

    if (!rooms.contains(port)) {
        qDebug() << "[RoomManager] Room not found on port" << port;
        return false;
    }

    Room& room = rooms[port];

    if (room.hasGuest()) {
        qDebug() << "[RoomManager] Room already has a guest.";
        return false;
    }

    if (room.isPasswordProtected() && room.password != password) {
        qDebug() << "[RoomManager] Password mismatch for room" << port;
        return false;
    }

    // Update room state
    room.guestUsername = guestUsername;
    room.guestSocket = guestSocket;
    room.status = RoomStatus::InProgress;

    qDebug() << "[RoomManager] Guest" << guestUsername << "joined room" << port;

    // --- FIX: Send MSG_PLAYER_JOINED to Host ---
    if (room.hostSocket && room.hostSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject notifyData;

        // کلید دقیقاً باید همان چیزی باشد که کلاینت‌منیجر می‌خواهد
        notifyData["playerName"] = guestUsername;

        QString notifyMsg = NetworkProtocol::buildMessage(NetworkConstants::MSG_PLAYER_JOINED, notifyData);

        // تضمین وجود کاراکتر پایان‌خط برای عبور از حلقه while در کلاینت‌منیجر
        if (!notifyMsg.endsWith('\n')) {
            notifyMsg += '\n';
        }

        room.hostSocket->write(notifyMsg.toUtf8());
        room.hostSocket->flush();
        qDebug() << "[RoomManager] Sent MSG_PLAYER_JOINED to host:" << notifyMsg.trimmed();
    } else {
        qDebug() << "[RoomManager] ERROR: Host socket invalid, cannot send notification";
    }

    if (outRoom) {
        *outRoom = room;
    }

    emit roomJoined(port, guestUsername);
    return true;
}

bool RoomManager::leaveRoom(quint16 port, const QString& username)
{
    QMutexLocker locker(&mutex);

    if (!rooms.contains(port)) {
        return false;
    }

    Room& room = rooms[port];

    if (room.hostUsername == username) {
        // Host leaves: remove the room and notify guest
        if (room.guestSocket) {
            QJsonObject data;
            data["message"] = "Host left the room.";
            QString msg = NetworkProtocol::buildMessage(NetworkConstants::MSG_ROOM_ERROR, data);
            room.guestSocket->write(msg.toUtf8());
            room.guestSocket->flush();
        }
        rooms.remove(port);
        emit roomClosed(port);
        qDebug() << "[RoomManager] Host left, room removed:" << port;
        return true;
    } else if (room.guestUsername == username) {
        // Guest leaves: clear guest and notify host
        if (room.hostSocket) {
            QJsonObject data;
            data["playerName"] = username;
            QString msg = NetworkProtocol::buildMessage(NetworkConstants::MSG_PLAYER_LEFT, data);
            room.hostSocket->write(msg.toUtf8());
            room.hostSocket->flush();
        }
        room.guestUsername.clear();
        room.guestSocket = nullptr;
        room.status = RoomStatus::Waiting;
        emit playerLeft(port, username);
        qDebug() << "[RoomManager] Guest left:" << username << "from room" << port;
        return true;
    }

    return false;
}

bool RoomManager::removeRoom(quint16 port)
{
    QMutexLocker locker(&mutex);
    if (rooms.contains(port)) {
        rooms.remove(port);
        emit roomClosed(port);
        return true;
    }
    return false;
}

Room* RoomManager::getRoom(quint16 port)
{
    QMutexLocker locker(&mutex);
    if (rooms.contains(port)) {
        return &rooms[port];
    }
    return nullptr;
}

QList<Room> RoomManager::getAllRooms() const
{
    QMutexLocker locker(&mutex);
    return rooms.values();
}