#include "roommanager.h"
#include "network_constants.h"
#include "network_protocol.h"
#include "../Game/GameController.h"
#include "../Game/BoardModel.h"
#include "../Game/Player.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QPoint>
#include <QDebug>

RoomManager::RoomManager(QObject *parent)
    : QObject(parent)
{
}

RoomManager::~RoomManager()
{
    cleanupRooms();
}

void RoomManager::cleanupRooms()
{
    QMutexLocker locker(&mutex);
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
        delete it.value();
    }
    rooms.clear();
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

    Room* room = new Room(roomName, port, hostUsername, hostSocket, settings, password);
    rooms.insert(port, room);

    qDebug() << "[RoomManager] Room created:" << roomName << "port:" << port << "host:" << hostUsername;

    if (outRoom) {
        outRoom->roomName = room->roomName;
        outRoom->port = room->port;
        outRoom->hostUsername = room->hostUsername;
        outRoom->hostSocket = room->hostSocket;
        outRoom->password = room->password;
        outRoom->gameSettings = room->gameSettings;
        outRoom->status = room->status;
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

    Room* room = rooms.value(port);
    if (!room) {
        return false;
    }

    if (room->hasGuest()) {
        qDebug() << "[RoomManager] Room already has a guest.";
        return false;
    }

    if (room->isPasswordProtected() && room->password != password) {
        qDebug() << "[RoomManager] Password mismatch for room" << port;
        return false;
    }

    room->guestUsername = guestUsername;
    room->guestSocket = guestSocket;
    room->status = RoomStatus::InProgress;

    qDebug() << "[RoomManager] Guest" << guestUsername << "joined room" << port;

    if (room->hostSocket && room->hostSocket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject notifyData;
        notifyData["playerName"] = guestUsername;
        QString notifyMsg = NetworkProtocol::buildMessage(NetworkConstants::MSG_PLAYER_JOINED, notifyData);
        if (!notifyMsg.endsWith('\n')) {
            notifyMsg += '\n';
        }
        room->hostSocket->write(notifyMsg.toUtf8());
        room->hostSocket->flush();
        qDebug() << "[RoomManager] Sent MSG_PLAYER_JOINED to host:" << notifyMsg.trimmed();
    } else {
        qDebug() << "[RoomManager] ERROR: Host socket invalid, cannot send notification";
    }

    if (outRoom) {
        outRoom->roomName = room->roomName;
        outRoom->port = room->port;
        outRoom->hostUsername = room->hostUsername;
        outRoom->hostSocket = room->hostSocket;
        outRoom->guestUsername = room->guestUsername;
        outRoom->guestSocket = room->guestSocket;
        outRoom->password = room->password;
        outRoom->gameSettings = room->gameSettings;
        outRoom->status = room->status;
        outRoom->gameStarted = room->gameStarted;
        outRoom->gameType = room->gameType;
        outRoom->currentPlayerId = room->currentPlayerId;
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

    Room* room = rooms.value(port);
    if (!room) {
        return false;
    }

    if (room->hostUsername == username) {
        if (room->guestSocket) {
            QJsonObject data;
            data["message"] = "Host left the room.";
            QString msg = NetworkProtocol::buildMessage(NetworkConstants::MSG_ROOM_ERROR, data);
            room->guestSocket->write(msg.toUtf8());
            room->guestSocket->flush();
        }
        delete room;
        rooms.remove(port);
        emit roomClosed(port);
        qDebug() << "[RoomManager] Host left, room removed:" << port;
        return true;
    } else if (room->guestUsername == username) {
        if (room->hostSocket) {
            QJsonObject data;
            data["playerName"] = username;
            QString msg = NetworkProtocol::buildMessage(NetworkConstants::MSG_PLAYER_LEFT, data);
            room->hostSocket->write(msg.toUtf8());
            room->hostSocket->flush();
        }
        room->guestUsername.clear();
        room->guestSocket = nullptr;
        room->status = RoomStatus::Waiting;
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
        delete rooms.value(port);
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
        return rooms.value(port);
    }
    return nullptr;
}

QList<Room*> RoomManager::getAllRooms() const
{
    QMutexLocker locker(&mutex);
    QList<Room*> result;
    for (auto it = rooms.begin(); it != rooms.end(); ++it) {
        if (it.value()) {
            result.append(it.value());
        }
    }
    return result;
}

bool RoomManager::startGame(quint16 port, int boardSize)
{
    QMutexLocker locker(&mutex);

    if (!rooms.contains(port)) {
        qDebug() << "[RoomManager] startGame: Room not found";
        return false;
    }

    Room* room = rooms.value(port);
    if (!room) {
        return false;
    }

    if (room->gameStarted) {
        qDebug() << "[RoomManager] startGame: Game already started";
        return false;
    }

    if (!room->hasGuest()) {
        qDebug() << "[RoomManager] startGame: No guest in room";
        return false;
    }

    // ===== تغییر: make_unique → make_shared =====
    room->gameController = std::make_shared<GameController>(boardSize, this);
    // ===== پایان تغییر =====

    room->gameController->setPlayerNames(room->hostUsername, room->guestUsername);

    const Player* p1 = room->gameController->getPlayer(0);
    const Player* p2 = room->gameController->getPlayer(1);
    if (p1) {
        const_cast<Player*>(p1)->setPlayerId(0);
    }
    if (p2) {
        const_cast<Player*>(p2)->setPlayerId(1);
    }

    room->gameStarted = true;
    room->status = RoomStatus::InProgress;
    room->currentPlayerId = 0;

    qDebug() << "[RoomManager] Game started in room" << port;
    return true;
}

bool RoomManager::processGameMove(quint16 port, const QPoint& p1, const QPoint& p2,
                                  int playerId, QVector<QPoint>& completedBoxes)
{
    QMutexLocker locker(&mutex);

    if (!rooms.contains(port)) {
        qDebug() << "[RoomManager] processGameMove: Room not found";
        return false;
    }

    Room* room = rooms.value(port);
    if (!room) {
        return false;
    }

    if (!room->gameController) {
        qDebug() << "[RoomManager] processGameMove: No game controller";
        return false;
    }

    if (room->gameController->isGameOver()) {
        qDebug() << "[RoomManager] processGameMove: Game is already over";
        return false;
    }

    if (room->currentPlayerId != playerId) {
        qDebug() << "[RoomManager] processGameMove: Not your turn";
        return false;
    }

    BoardModel* model = const_cast<BoardModel*>(room->gameController->getModel());
    if (!model) {
        qDebug() << "[RoomManager] processGameMove: No model";
        return false;
    }

    if (!model->isValidMove(p1, p2)) {
        qDebug() << "[RoomManager] processGameMove: Invalid move";
        return false;
    }

    completedBoxes = model->makeMove(p1, p2, playerId);

    if (!completedBoxes.isEmpty()) {
        const Player* player = room->gameController->getPlayer(playerId);
        if (player) {
            const_cast<Player*>(player)->addScore(completedBoxes.size());
            qDebug() << "[RoomManager] Player" << playerId << "scored" << completedBoxes.size() << "boxes";
        }
    } else {
        room->currentPlayerId = (room->currentPlayerId == 0) ? 1 : 0;
        qDebug() << "[RoomManager] Turn switched to player" << room->currentPlayerId;
    }

    if (room->gameController->isGameOver()) {
        room->status = RoomStatus::Finished;
        qDebug() << "[RoomManager] Game over in room" << port;
    }

    return true;
}

bool RoomManager::endGame(quint16 port, int& winner, int& p1Score, int& p2Score)
{
    QMutexLocker locker(&mutex);

    if (!rooms.contains(port)) {
        return false;
    }

    Room* room = rooms.value(port);
    if (!room) {
        return false;
    }

    if (!room->gameController) {
        return false;
    }

    winner = room->gameController->getWinner();

    const Player* p1 = room->gameController->getPlayer(0);
    const Player* p2 = room->gameController->getPlayer(1);
    p1Score = p1 ? p1->getScore() : 0;
    p2Score = p2 ? p2->getScore() : 0;

    room->status = RoomStatus::Finished;
    room->gameStarted = false;

    return true;
}

QJsonObject RoomManager::getGameState(quint16 port)
{
    QMutexLocker locker(&mutex);
    QJsonObject state;

    if (!rooms.contains(port)) {
        return state;
    }

    Room* room = rooms.value(port);
    if (!room) {
        return state;
    }

    if (!room->gameController) {
        return state;
    }

    const BoardModel* model = room->gameController->getModel();
    if (!model) {
        return state;
    }

    int size = model->getBoardSize();

    QJsonArray horizontalLines;
    QJsonArray verticalLines;
    QJsonArray boxes;

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

    const Player* p1 = room->gameController->getPlayer(0);
    const Player* p2 = room->gameController->getPlayer(1);

    state["boardSize"] = size;
    state["horizontalLines"] = horizontalLines;
    state["verticalLines"] = verticalLines;
    state["boxes"] = boxes;
    state["player1Score"] = p1 ? p1->getScore() : 0;
    state["player2Score"] = p2 ? p2->getScore() : 0;
    state["currentPlayer"] = room->currentPlayerId;
    state["gameOver"] = room->gameController->isGameOver();

    return state;
}