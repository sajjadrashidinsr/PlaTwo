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

bool RoomManager::startGame(quint16 port, int boardSize)
{
    QMutexLocker locker(&mutex);
    
    if (!rooms.contains(port)) return false;
    Room& room = rooms[port];
    if (room.gameStarted) return false;
    if (!room.hasGuest()) return false;
    
    room.gameController = std::make_unique<GameController>(boardSize, this);
    room.gameController->setPlayerNames(room.hostUsername, room.guestUsername);
    room.gameController->getPlayer(0)->setPlayerId(0);
    room.gameController->getPlayer(1)->setPlayerId(1);
    
    room.gameStarted = true;
    room.status = RoomStatus::InProgress;
    room.currentPlayerId = 0;
    
    return true;
}

bool RoomManager::processGameMove(quint16 port, const QPoint& p1, const QPoint& p2,
                                  int playerId, QVector<QPoint>& completedBoxes)
{
    QMutexLocker locker(&mutex);
    
    if (!rooms.contains(port)) return false;
    Room& room = rooms[port];
    if (!room.gameController) return false;
    if (room.gameController->isGameOver()) return false;
    if (room.currentPlayerId != playerId) return false;
    
    const BoardModel* model = room.gameController->getModel();
    if (!model->isValidMove(p1, p2)) return false;
    
    completedBoxes = room.gameController->getModel()->makeMove(p1, p2, playerId);
    
    if (!completedBoxes.isEmpty()) {
        Player* player = room.gameController->getPlayer(playerId);
        if (player) player->addScore(completedBoxes.size());
    } else {
        room.currentPlayerId = (room.currentPlayerId == 0) ? 1 : 0;
    }
    
    if (room.gameController->isGameOver()) {
        room.status = RoomStatus::Finished;
    }
    
    return true;
}

bool RoomManager::endGame(quint16 port, int& winner, int& p1Score, int& p2Score)
{
    QMutexLocker locker(&mutex);
    if (!rooms.contains(port)) return false;
    Room& room = rooms[port];
    if (!room.gameController) return false;
    
    winner = room.gameController->getWinner();
    p1Score = room.gameController->getPlayer(0)->getScore();
    p2Score = room.gameController->getPlayer(1)->getScore();
    
    room.status = RoomStatus::Finished;
    room.gameStarted = false;
    
    return true;
}

QJsonObject RoomManager::getGameState(quint16 port)
{
    QMutexLocker locker(&mutex);
    QJsonObject state;
    if (!rooms.contains(port)) return state;
    Room& room = rooms[port];
    if (!room.gameController) return state;
    
    const BoardModel* model = room.gameController->getModel();
    int size = model->getBoardSize();
    
    QJsonArray horizontalLines, verticalLines, boxes;
    
    for (int row = 0; row < size; ++row) {
        QJsonArray rowData;
        for (int col = 0; col < size - 1; ++col) {
            rowData.append(model->getLine(row, col, BoardModel::LineType::Horizontal));
        }
        horizontalLines.append(rowData);
    }
    
    for (int row = 0; row < size - 1; ++row) {
        QJsonArray rowData;
        for (int col = 0; col < size; ++col) {
            rowData.append(model->getLine(row, col, BoardModel::LineType::Vertical));
        }
        verticalLines.append(rowData);
    }
    
    for (int row = 0; row < size - 1; ++row) {
        QJsonArray rowData;
        for (int col = 0; col < size - 1; ++col) {
            rowData.append(model->getBox(row, col));
        }
        boxes.append(rowData);
    }
    
    state["boardSize"] = size;
    state["horizontalLines"] = horizontalLines;
    state["verticalLines"] = verticalLines;
    state["boxes"] = boxes;
    state["player1Score"] = room.gameController->getPlayer(0)->getScore();
    state["player2Score"] = room.gameController->getPlayer(1)->getScore();
    state["currentPlayer"] = room.currentPlayerId;
    state["gameOver"] = room.gameController->isGameOver();
    
    return state;
}