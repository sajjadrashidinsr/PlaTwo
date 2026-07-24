#include "request_worker.h"
#include <QMetaObject>
#include <QDebug>
#include <QThread>
#include <QPoint>
#include <QDateTime>
#include "../Game/GameController.h"

RequestWorker::RequestWorker(QTcpSocket* clientSocket,
                             const QString& message,
                             std::shared_ptr<storage_manager> storage,
                             std::shared_ptr<RoomManager> roomManager,
                             QObject* parent)
    : QRunnable()
    , socket(clientSocket)
    , messageBuffer(message)
    , storageManager(storage)
    , roomManager(roomManager)
    , parentObject(parent) {

    setAutoDelete(true);
}


RequestWorker::~RequestWorker() {
    qDebug() << "[Worker] Destroyed";
}

void RequestWorker::run() {
    if (!socket) {
        qDebug() << "[Worker] Socket already destroyed";
        return;
    }

    if (socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "[Worker] Socket not connected";
        return;
    }

    processMessage(messageBuffer);
}


void RequestWorker::processMessage(const QString& message) {
    int type;
    QJsonObject data;

    if (!NetworkProtocol::parseMessage(message, type, data)) {
        qDebug() << "[Worker] Invalid message format";
        sendResponse(NetworkConstants::MSG_ERROR, false, "Invalid message format");
        return;
    }

    qDebug() << "[Worker] Message type:" << type;

    switch (type) {
    case NetworkConstants::MSG_REGISTER:
        handleRegister(data);
        break;
    case NetworkConstants::MSG_LOGIN:
        handleLogin(data);
        break;
    case NetworkConstants::MSG_FORGOT_PASSWORD:
        handleForgotPassword(data);
        break;
    case NetworkConstants::MSG_CHANGE_PASSWORD:
        handleChangePassword(data);
        break;
    case NetworkConstants::MSG_GET_USER:
        handleGetUser(data);
        break;
    case NetworkConstants::MSG_UPDATE_USER:
        handleUpdateUser(data);
        break;
    case NetworkConstants::MSG_CREATE_ROOM:
        handleCreateRoom(data);
        break;
    case NetworkConstants::MSG_JOIN_ROOM:
        handleJoinRoom(data);
        break;
    case NetworkConstants::MSG_LEAVE_ROOM:
        handleLeaveRoom(data);
        break;
    case NetworkConstants::MSG_GAME_START:
        handleGameStart(data);
        break;
    case NetworkConstants::MSG_GAME_MOVE:
        handleGameMove(data);
        break;
    case NetworkConstants::MSG_GAME_ABORTED:
        handleGameAbort(data);
        break;
    case NetworkConstants::MSG_GAME_READY:
        handleGameReady(data);
        break;
    default:
        sendResponse(NetworkConstants::MSG_ERROR, false, "Unknown message type");
        break;
    }
}

void RequestWorker::sendResponse(int type, bool success,
                                 const QString& message, const QJsonObject& data) {
    if (!socket) {
        qDebug() << "[Worker] Socket is null, cannot send response";
        return;
    }

    if (socket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "[Worker] Socket not connected, cannot send response";
        return;
    }

    QJsonObject responseData = data;
    responseData["success"] = success;
    responseData["message"] = message;

    QString response = NetworkProtocol::buildMessage(type, responseData);

    socket->write(response.toUtf8());
    socket->flush();
}

void RequestWorker::handleRegister(const QJsonObject& data) {
    qDebug() << "[Worker] handleRegister called";

    if (!data.contains("username") || !data.contains("passwordHash") ||
        !data.contains("name") || !data.contains("phone") || !data.contains("email")) {
        sendResponse(NetworkConstants::MSG_REGISTER, false, "Missing required fields");
        return;
    }

    user newUser;
    newUser.username = data["username"].toString();
    newUser.name = data["name"].toString();
    newUser.phone = data["phone"].toString();
    newUser.email = data["email"].toString();
    QString rawPassword = data["passwordHash"].toString();
    newUser.passwordHash = AuthManager::hashPassword(rawPassword);

    qDebug() << "[Worker] Registering user:" << newUser.username;

    if (!AuthManager::validateEmail(newUser.email)) {
        sendResponse(NetworkConstants::MSG_REGISTER, false, "Invalid email format");
        return;
    }

    if (!AuthManager::validatePhone(newUser.phone)) {
        sendResponse(NetworkConstants::MSG_REGISTER, false, "Invalid phone number");
        return;
    }

    if (rawPassword.length() < 8) {
        sendResponse(NetworkConstants::MSG_REGISTER, false, "Password must be at least 8 characters");
        return;
    }

    newUser.passwordHash = AuthManager::hashPassword(rawPassword);
    if (newUser.passwordHash.isEmpty()) {
        sendResponse(NetworkConstants::MSG_REGISTER, false, "Password hashing failed");
        return;
    }

    qDebug() << "[Worker] Attempting to save user to database...";

    if (storageManager->registeruser(newUser)) {
        sendResponse(NetworkConstants::MSG_REGISTER, true, "Registration successful");
    } else {
        sendResponse(NetworkConstants::MSG_REGISTER, false, "Username already exists or database error");
    }
}

void RequestWorker::handleLogin(const QJsonObject& data) {
    qDebug() << "[Worker] handleLogin called";

    QString username = data["username"].toString();
    QString password = data["password"].toString();

    if (username.isEmpty() || password.isEmpty()) {
        sendResponse(NetworkConstants::MSG_LOGIN, false, "Username and password required");
        return;
    }

    qDebug() << "[Worker] Login attempt for:" << username;

    user* u = storageManager->getuser(username);
    if (!u) {
        qDebug() << "[Worker] User not found:" << username;
        sendResponse(NetworkConstants::MSG_LOGIN, false, "User not found");
        return;
    }

    QString hashedInput = AuthManager::hashPassword(password);

    if (u->passwordHash == hashedInput) {
        socket->setProperty("username", username);

        QJsonObject responseData;
        responseData["user"] = NetworkProtocol::userToJson(*u);
        sendResponse(NetworkConstants::MSG_LOGIN, true, "Login successful", responseData);
        delete u;
    } else {
        qDebug() << "[Worker] Invalid password for:" << username;
        sendResponse(NetworkConstants::MSG_LOGIN, false, "Invalid password");
        delete u;
    }
}

void RequestWorker::handleForgotPassword(const QJsonObject& data) {
    QString username = data["username"].toString();
    QString phone = data["phone"].toString();

    if (username.isEmpty() || phone.isEmpty()) {
        sendResponse(NetworkConstants::MSG_FORGOT_PASSWORD, false, "Username and phone required");
        return;
    }

    user* u = storageManager->getuser(username);
    if (!u) {
        sendResponse(NetworkConstants::MSG_FORGOT_PASSWORD, false, "User not found");
        return;
    }

    if (AuthManager::verifyPhoneForRecovery(u->phone, phone)) {
        sendResponse(NetworkConstants::MSG_FORGOT_PASSWORD, true, "Phone number verified");
    } else {
        sendResponse(NetworkConstants::MSG_FORGOT_PASSWORD, false, "Phone number does not match");
    }
    delete u;
}

void RequestWorker::handleChangePassword(const QJsonObject& data) {
    QString username = data["username"].toString();
    QString phone = data["phone"].toString();
    QString newPassword = data["newPassword"].toString();

    if (username.isEmpty() || phone.isEmpty() || newPassword.isEmpty()) {
        sendResponse(NetworkConstants::MSG_CHANGE_PASSWORD, false, "All fields required");
        return;
    }

    if (newPassword.length() < 8) {
        sendResponse(NetworkConstants::MSG_CHANGE_PASSWORD, false, "Password must be at least 8 characters");
        return;
    }

    user* u = storageManager->getuser(username);
    if (!u) {
        sendResponse(NetworkConstants::MSG_CHANGE_PASSWORD, false, "User not found");
        return;
    }

    if (!AuthManager::verifyPhoneForRecovery(u->phone, phone)) {
        sendResponse(NetworkConstants::MSG_CHANGE_PASSWORD, false, "Phone number does not match");
        delete u;
        return;
    }

    u->passwordHash = AuthManager::hashPassword(newPassword);
    if (u->passwordHash.isEmpty()) {
        sendResponse(NetworkConstants::MSG_CHANGE_PASSWORD, false, "Password hashing failed");
        delete u;
        return;
    }

    if (storageManager->updateuser(*u)) {
        sendResponse(NetworkConstants::MSG_CHANGE_PASSWORD, true, "Password changed successfully");
    } else {
        sendResponse(NetworkConstants::MSG_CHANGE_PASSWORD, false, "Failed to update password");
    }
    delete u;
}

void RequestWorker::handleGetUser(const QJsonObject& data) {
    QString username = data["username"].toString();

    if (username.isEmpty()) {
        sendResponse(NetworkConstants::MSG_GET_USER, false, "Username required");
        return;
    }

    user* u = storageManager->getuser(username);
    if (!u) {
        sendResponse(NetworkConstants::MSG_GET_USER, false, "User not found");
        return;
    }

    QJsonObject responseData;
    responseData["user"] = NetworkProtocol::userToJson(*u);
    sendResponse(NetworkConstants::MSG_GET_USER, true, "User data retrieved", responseData);
    delete u;
}

void RequestWorker::handleUpdateUser(const QJsonObject& data) {
    if (!data.contains("oldUsername") || !data.contains("user")) {
        sendResponse(NetworkConstants::MSG_UPDATE_USER, false, "Missing oldUsername or user data");
        return;
    }

    QString oldUsername = data["oldUsername"].toString();
    QJsonObject userObj = data["user"].toObject();
    user updatedUser = NetworkProtocol::userFromJson(userObj);

    qDebug() << "[Worker] Updating user profile. Old username:" << oldUsername << ", New username:" << updatedUser.username;

    if (updatedUser.passwordHash.isEmpty()) {
        user* existingUser = storageManager->getuser(oldUsername);
        if (existingUser) {
            updatedUser.passwordHash = existingUser->passwordHash;
            delete existingUser;
        } else {
            sendResponse(NetworkConstants::MSG_UPDATE_USER, false, "User not found in database.");
            return;
        }
    } else {
        updatedUser.passwordHash = AuthManager::hashPassword(updatedUser.passwordHash);
    }

    if (storageManager->updateuser(oldUsername, updatedUser)) {
        sendResponse(NetworkConstants::MSG_UPDATE_USER, true, "User updated successfully");
    } else {
        sendResponse(NetworkConstants::MSG_UPDATE_USER, false, "Failed to update user. Username might be taken.");
    }
}

void RequestWorker::handleCreateRoom(const QJsonObject& data) {
    qDebug() << "[Worker] handleCreateRoom called";

    QString username = socket->property("username").toString();
    if (username.isEmpty()) {
        sendResponse(NetworkConstants::MSG_ROOM_ERROR, false, "You must be logged in to create a room.");
        return;
    }

    if (!data.contains("roomName") || !data.contains("port") ||
        !data.contains("gameSettings")) {
        sendResponse(NetworkConstants::MSG_ROOM_ERROR, false, "Missing room creation data");
        return;
    }

    QString roomName = data["roomName"].toString();
    quint16 port = static_cast<quint16>(data["port"].toInt());
    GameSettings settings = NetworkProtocol::gameSettingsFromJson(data["gameSettings"].toObject());
    QString password = data["password"].toString();

    Room createdRoom;
    if (roomManager->createRoom(roomName, port, username, socket, settings, password, &createdRoom)) {
        QJsonObject responseData;
        responseData["room"] = NetworkProtocol::roomToJson(createdRoom);
        sendResponse(NetworkConstants::MSG_ROOM_CREATED, true, "Room created successfully", responseData);
    } else {
        sendResponse(NetworkConstants::MSG_ROOM_ERROR, false, "Failed to create room. Port may be in use.");
    }
}

void RequestWorker::handleJoinRoom(const QJsonObject& data) {
    qDebug() << "[Worker] handleJoinRoom called";

    QString username = socket->property("username").toString();
    if (username.isEmpty()) {
        sendResponse(NetworkConstants::MSG_ROOM_ERROR, false, "You must be logged in to join a room.");
        return;
    }

    if (!data.contains("ip") || !data.contains("port")) {
        sendResponse(NetworkConstants::MSG_ROOM_ERROR, false, "Missing join room data");
        return;
    }

    quint16 port = static_cast<quint16>(data["port"].toInt());
    QString password = data["password"].toString();

    Room joinedRoom;
    if (roomManager->joinRoom(port, username, socket, password, &joinedRoom)) {
        QJsonObject responseData;
        responseData["room"] = NetworkProtocol::roomToJson(joinedRoom);
        sendResponse(NetworkConstants::MSG_ROOM_JOINED, true, "Joined room successfully", responseData);

        Room* room = roomManager->getRoom(port);
        if (room && room->hostSocket) {
            QJsonObject notifyData;
            notifyData["playerName"] = username;
            QString notifyMsg = NetworkProtocol::buildMessage(NetworkConstants::MSG_PLAYER_JOINED, notifyData);
            room->hostSocket->write(notifyMsg.toUtf8());
            room->hostSocket->flush();
            qDebug() << "[Worker] Notified host about guest join";
        }
    } else {
        sendResponse(NetworkConstants::MSG_ROOM_ERROR, false, "Failed to join room. Room not found, full, or password incorrect.");
    }
}

void RequestWorker::handleLeaveRoom(const QJsonObject& data) {
    qDebug() << "[Worker] handleLeaveRoom called";

    QString username = socket->property("username").toString();
    if (username.isEmpty()) {
        sendResponse(NetworkConstants::MSG_ROOM_ERROR, false, "You must be logged in to leave a room.");
        return;
    }

    if (!data.contains("port")) {
        sendResponse(NetworkConstants::MSG_ROOM_ERROR, false, "Missing port for leave room");
        return;
    }

    quint16 port = static_cast<quint16>(data["port"].toInt());

    if (roomManager->leaveRoom(port, username)) {
        sendResponse(NetworkConstants::MSG_SUCCESS, true, "Left room successfully");
    } else {
        sendResponse(NetworkConstants::MSG_ROOM_ERROR, false, "Failed to leave room. You may not be in this room.");
    }
}

void RequestWorker::handleGameStart(const QJsonObject& data)
{
    if (!data.contains("port")) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Missing port");
        return;
    }

    quint16 port = static_cast<quint16>(data["port"].toInt());
    int boardSize = data.contains("boardSize") ? data["boardSize"].toInt() : 5;

    QString username = socket->property("username").toString();
    if (username.isEmpty()) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Not logged in");
        return;
    }

    Room* room = roomManager->getRoom(port);
    if (!room) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Room not found");
        return;
    }

    if (room->hostUsername != username) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Only host can start the game");
        return;
    }

    if (!room->hasGuest()) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Need a guest to start");
        return;
    }

    if (roomManager->startGame(port, boardSize)) {
        QJsonObject responseData;
        responseData["boardSize"] = boardSize;
        responseData["hostUsername"] = room->hostUsername;
        responseData["guestUsername"] = room->guestUsername;
        responseData["status"] = "started";
        responseData["port"] = static_cast<int>(port);

        QJsonObject hostData = responseData;
        hostData["isHost"] = true;
        QString hostMessage = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_START, hostData);

        if (room->hostSocket) {
            room->hostSocket->write(hostMessage.toUtf8());
            room->hostSocket->flush();
            qDebug() << "[Worker] Sent MSG_GAME_START to host (isHost=true)";
        }

        QJsonObject guestData = responseData;
        guestData["isHost"] = false;
        QString guestMessage = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_START, guestData);

        if (room->guestSocket) {
            room->guestSocket->write(guestMessage.toUtf8());
            room->guestSocket->flush();
            qDebug() << "[Worker] Sent MSG_GAME_START to guest (isHost=false)";
        }

        QJsonObject stateData;
        stateData["port"] = static_cast<int>(port);
        stateData["state"] = roomManager->getGameState(port);
        QString stateMsg = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_STATE, stateData);

        if (room->hostSocket) {
            room->hostSocket->write(stateMsg.toUtf8());
            room->hostSocket->flush();
            qDebug() << "[Worker] Sent MSG_GAME_STATE to host";
        }
        if (room->guestSocket) {
            room->guestSocket->write(stateMsg.toUtf8());
            room->guestSocket->flush();
            qDebug() << "[Worker] Sent MSG_GAME_STATE to guest";
        }

    } else {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Failed to start game");
    }
}

void RequestWorker::handleGameMove(const QJsonObject& data)
{
    if (!data.contains("port") || !data.contains("x1") || !data.contains("y1") ||
        !data.contains("x2") || !data.contains("y2") || !data.contains("playerId")) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Invalid move data");
        return;
    }

    quint16 port = static_cast<quint16>(data["port"].toInt());
    QPoint p1(data["x1"].toInt(), data["y1"].toInt());
    QPoint p2(data["x2"].toInt(), data["y2"].toInt());
    int playerId = data["playerId"].toInt();

    QString username = socket->property("username").toString();
    if (username.isEmpty()) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Not logged in");
        return;
    }

    Room* room = roomManager->getRoom(port);
    if (!room) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Room not found");
        return;
    }

    if (room->hostUsername != username && room->guestUsername != username) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "You are not in this room");
        return;
    }

    int expectedPlayerId = (room->hostUsername == username) ? 0 : 1;
    if (expectedPlayerId != playerId) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Invalid player ID");
        return;
    }

    if (room->currentPlayerId != playerId) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Not your turn");
        return;
    }

    QVector<QPoint> completedBoxes;
    bool success = roomManager->processGameMove(port, p1, p2, playerId, completedBoxes);

    if (!success) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Invalid move");
        return;
    }

    // ارسال وضعیت جدید به هر دو کلاینت
    QJsonObject stateData;
    stateData["port"] = static_cast<int>(port);
    stateData["state"] = roomManager->getGameState(port);

    QString stateMsg = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_STATE, stateData);

    if (room->hostSocket) {
        room->hostSocket->write(stateMsg.toUtf8());
        room->hostSocket->flush();
    }
    if (room->guestSocket) {
        room->guestSocket->write(stateMsg.toUtf8());
        room->guestSocket->flush();
    }

    // بررسی پایان بازی
    if (room->gameController && room->gameController->isGameOver()) {
        int winner, p1Score, p2Score;
        roomManager->endGame(port, winner, p1Score, p2Score);

        QJsonObject gameOverData;
        gameOverData["port"] = static_cast<int>(port);
        gameOverData["winner"] = winner;
        gameOverData["player1Score"] = p1Score;
        gameOverData["player2Score"] = p2Score;

        QString gameOverMsg = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_OVER, gameOverData);

        if (room->hostSocket) {
            room->hostSocket->write(gameOverMsg.toUtf8());
            room->hostSocket->flush();
        }
        if (room->guestSocket) {
            room->guestSocket->write(gameOverMsg.toUtf8());
            room->guestSocket->flush();
        }

        // ذخیره تاریخچه در دیتابیس
        user* host = storageManager->getuser(room->hostUsername);
        user* guest = storageManager->getuser(room->guestUsername);

        if (host && guest) {
            // به‌روزرسانی امتیازها
            if (winner == 0) {
                host->dotsAndBoxesScore += 10;
            } else if (winner == 1) {
                guest->dotsAndBoxesScore += 10;
            }

            // تاریخچه Host
            GameRecord hostRecord;
            hostRecord.opponentUsername = guest->username;
            hostRecord.date = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm");
            hostRecord.role = "Host";
            hostRecord.winner = (winner == 0) ? host->username : (winner == 1) ? guest->username : "Draw";
            hostRecord.finalScore = (winner == 0) ? p1Score : p2Score;

            // تاریخچه Guest
            GameRecord guestRecord;
            guestRecord.opponentUsername = host->username;
            guestRecord.date = hostRecord.date;
            guestRecord.role = "Guest";
            guestRecord.winner = hostRecord.winner;
            guestRecord.finalScore = (winner == 1) ? p2Score : p1Score;

            // ذخیره در دیتابیس
            storageManager->updateuser(*host);
            storageManager->updateuser(*guest);
            storageManager->addGameRecord(host->username, hostRecord);
            storageManager->addGameRecord(guest->username, guestRecord);

            delete host;
            delete guest;
        }

        qDebug() << "[Game] Game over in room" << port << "Winner:" << winner;
    }

    sendResponse(NetworkConstants::MSG_SUCCESS, true, "Move processed");
}

void RequestWorker::handleGameAbort(const QJsonObject& data)
{
    if (!data.contains("port")) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Missing port");
        return;
    }

    quint16 port = static_cast<quint16>(data["port"].toInt());
    QString username = socket->property("username").toString();

    Room* room = roomManager->getRoom(port);
    if (!room) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Room not found");
        return;
    }

    QJsonObject abortData;
    abortData["message"] = username + " has left the game";
    QString abortMsg = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_ABORTED, abortData);

    if (room->hostUsername != username && room->guestSocket) {
        room->guestSocket->write(abortMsg.toUtf8());
        room->guestSocket->flush();
    } else if (room->guestUsername != username && room->hostSocket) {
        room->hostSocket->write(abortMsg.toUtf8());
        room->hostSocket->flush();
    }

    room->gameController = nullptr;
    room->gameStarted = false;
    room->status = RoomStatus::Waiting;

    sendResponse(NetworkConstants::MSG_GAME_ABORTED, true, "Game aborted");
}

void RequestWorker::handleGameReady(const QJsonObject& data)
{
    if (!data.contains("port") || !data.contains("ready")) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Missing data");
        return;
    }

    quint16 port = static_cast<quint16>(data["port"].toInt());
    bool ready = data["ready"].toBool();
    QString username = socket->property("username").toString();

    Room* room = roomManager->getRoom(port);
    if (!room) {
        sendResponse(NetworkConstants::MSG_GAME_ERROR, false, "Room not found");
        return;
    }

    QJsonObject readyData;
    readyData["playerName"] = username;
    readyData["ready"] = ready;
    QString readyMsg = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_READY, readyData);

    if (room->hostUsername != username && room->guestSocket) {
        room->guestSocket->write(readyMsg.toUtf8());
        room->guestSocket->flush();
    } else if (room->guestUsername != username && room->hostSocket) {
        room->hostSocket->write(readyMsg.toUtf8());
        room->hostSocket->flush();
    }

    sendResponse(NetworkConstants::MSG_SUCCESS, true, "Ready status sent");
}