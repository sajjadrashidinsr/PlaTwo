#include "client_manager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ClientManager::ClientManager(QObject* parent)
    : QObject(parent)
    , socket(nullptr)
    , connectionTimer(nullptr)
    , awaitingResponse(false)
    , pendingPort(0) {

    socket = new QTcpSocket(this);

    connectionTimer = new QTimer(this);
    connectionTimer->setSingleShot(true);
    connectionTimer->setInterval(3000);

    connect(socket, &QTcpSocket::connected, this, &ClientManager::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &ClientManager::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &ClientManager::onReadyRead);
    connect(socket, &QTcpSocket::errorOccurred,
            this, &ClientManager::onError);
    connect(connectionTimer, &QTimer::timeout, this, &ClientManager::onConnectionTimeout);
}

ClientManager::~ClientManager() {
    disconnectFromServer();
}

// ---------- Connection ----------
bool ClientManager::connectToServer(const QString& address, quint16 port) {
    if (isConnected()) {
        return true;
    }

    socket->connectToHost(address, port);
    connectionTimer->start();
    return true;
}

void ClientManager::disconnectFromServer() {
    if (socket) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected(1000);
        }
    }
}

// ---------- Authentication ----------
void ClientManager::sendRegister(const user& newUser) {
    if (!isConnected()) {
        emit error("Not connected to server");
        return;
    }

    QJsonObject userObj = NetworkProtocol::userToJson(newUser);
    userObj.remove("history");

    QString message = NetworkProtocol::buildMessage(
        NetworkConstants::MSG_REGISTER,
        userObj
        );

    socket->write(message.toUtf8());
    awaitingResponse = true;
}

void ClientManager::sendLogin(const QString& username, const QString& password) {
    if (!isConnected()) {
        emit error("Not connected to server");
        return;
    }

    QJsonObject data;
    data["username"] = username;
    data["password"] = password;

    QString message = NetworkProtocol::buildMessage(
        NetworkConstants::MSG_LOGIN,
        data
        );

    socket->write(message.toUtf8());
    awaitingResponse = true;
}

void ClientManager::sendForgotPassword(const QString& username, const QString& phone) {
    if (!isConnected()) {
        emit error("Not connected to server");
        return;
    }

    QJsonObject data;
    data["username"] = username;
    data["phone"] = phone;

    QString message = NetworkProtocol::buildMessage(
        NetworkConstants::MSG_FORGOT_PASSWORD,
        data
        );

    socket->write(message.toUtf8());
    awaitingResponse = true;
}

void ClientManager::sendChangePassword(const QString& username, const QString& phone,
                                       const QString& newPassword) {
    if (!isConnected()) {
        emit error("Not connected to server");
        return;
    }

    QJsonObject data;
    data["username"] = username;
    data["phone"] = phone;
    data["newPassword"] = newPassword;

    QString message = NetworkProtocol::buildMessage(
        NetworkConstants::MSG_CHANGE_PASSWORD,
        data
        );

    socket->write(message.toUtf8());
    awaitingResponse = true;
}

void ClientManager::sendGetUser(const QString& username) {
    if (!isConnected()) {
        emit error("Not connected to server");
        return;
    }

    QJsonObject data;
    data["username"] = username;

    QString message = NetworkProtocol::buildMessage(
        NetworkConstants::MSG_GET_USER,
        data
        );

    socket->write(message.toUtf8());
    awaitingResponse = true;
}

void ClientManager::sendUpdateUser(const QString& oldUsername, const user& updatedUser) {
    if (!isConnected()) {
        emit error("Not connected to server");
        return;
    }

    QJsonObject data;
    data["oldUsername"] = oldUsername;
    data["user"] = NetworkProtocol::userToJson(updatedUser);

    QString message = NetworkProtocol::buildMessage(
        NetworkConstants::MSG_UPDATE_USER,
        data
        );

    socket->write(message.toUtf8());
    awaitingResponse = true;
}

// ---------- Room Management (Phase 2) ----------
void ClientManager::createRoom(const QString& roomName, quint16 port,
                               const GameSettings& settings, const QString& password) {
    if (!isConnected()) {
        emit roomError("Not connected to server");
        return;
    }

    // Store pending info for response handling
    pendingRoomName = roomName;
    pendingPort = port;
    pendingSettings = settings;
    pendingPassword = password;

    QJsonObject data;
    data["roomName"] = roomName;
    data["port"] = static_cast<int>(port);
    data["gameSettings"] = NetworkProtocol::gameSettingsToJson(settings);
    data["password"] = password;

    QString message = NetworkProtocol::buildMessage(
        NetworkConstants::MSG_CREATE_ROOM,
        data
        );

    socket->write(message.toUtf8());
    awaitingResponse = true;
    qDebug() << "[Client] Create room request sent:" << roomName << "port:" << port;
}

void ClientManager::joinRoom(const QString& ip, quint16 port, const QString& password) {
    if (!isConnected()) {
        emit roomError("Not connected to server");
        return;
    }

    QJsonObject data;
    data["ip"] = ip;
    data["port"] = static_cast<int>(port);
    data["password"] = password;

    QString message = NetworkProtocol::buildMessage(
        NetworkConstants::MSG_JOIN_ROOM,
        data
        );

    socket->write(message.toUtf8());
    awaitingResponse = true;
    qDebug() << "[Client] Join room request sent to" << ip << ":" << port;
}

void ClientManager::leaveRoom(quint16 port) {
    if (!isConnected()) {
        qDebug() << "[Client] Not connected, cannot leave room";
        return;
    }

    QJsonObject data;
    data["port"] = static_cast<int>(port);
    QString message = NetworkProtocol::buildMessage(NetworkConstants::MSG_LEAVE_ROOM, data);
    socket->write(message.toUtf8());
    qDebug() << "[Client] Leave room request sent for port:" << port;
}

// ---------- Slots ----------
void ClientManager::onConnected() {
    connectionTimer->stop();
    awaitingResponse = false;
    emit connected();
}

void ClientManager::onDisconnected() {
    awaitingResponse = false;
    emit disconnected();
}

void ClientManager::onError(QAbstractSocket::SocketError socketError) {
    Q_UNUSED(socketError)
    QString errorMsg = socket->errorString();
    emit error(errorMsg);
}

void ClientManager::onConnectionTimeout() {
    if (socket->state() != QAbstractSocket::ConnectedState) {
        socket->abort();
        emit error("Connection timeout");
    }
}

void ClientManager::onReadyRead() {
    buffer += QString::fromUtf8(socket->readAll());

    while (buffer.contains('\n')) {
        int separatorPos = buffer.indexOf('\n');
        QString message = buffer.left(separatorPos);
        buffer.remove(0, separatorPos + 1);

        if (!message.isEmpty()) {
            processMessage(message);
        }
    }
}

void ClientManager::processMessage(const QString& message) {
    int type;
    QJsonObject data;

    if (!NetworkProtocol::parseMessage(message, type, data)) {
        emit error("Invalid message from server");
        return;
    }

    awaitingResponse = false;

    switch (type) {
    // Authentication responses (unchanged)
    case NetworkConstants::MSG_REGISTER:
    case NetworkConstants::MSG_REGISTER_SUCCESS: {
        bool success = data["success"].toBool(false);
        QString msg = data["message"].toString();
        emit registerResponse(success, msg);
        break;
    }
    case NetworkConstants::MSG_LOGIN: {
        bool success = data["success"].toBool(false);
        QString msg = data["message"].toString();
        if (success && data.contains("user")) {
            user* u = new user(NetworkProtocol::userFromJson(data["user"].toObject()));
            emit loginResponse(success, u, msg);
        } else {
            emit loginResponse(success, nullptr, msg);
        }
        break;
    }
    case NetworkConstants::MSG_FORGOT_PASSWORD: {
        bool success = data["success"].toBool(false);
        QString msg = data["message"].toString();
        emit forgotPasswordResponse(success, msg);
        break;
    }
    case NetworkConstants::MSG_CHANGE_PASSWORD:
    case NetworkConstants::MSG_PASSWORD_CHANGED: {
        bool success = data["success"].toBool(false);
        QString msg = data["message"].toString();
        emit passwordChangedResponse(success, msg);
        break;
    }
    case NetworkConstants::MSG_GET_USER: {
        bool success = data["success"].toBool(false);
        QString msg = data["message"].toString();
        if (success && data.contains("user")) {
            user* u = new user(NetworkProtocol::userFromJson(data["user"].toObject()));
            emit getUserResponse(success, u, msg);
        } else {
            emit getUserResponse(success, nullptr, msg);
        }
        break;
    }
    case NetworkConstants::MSG_UPDATE_USER: {
        bool success = data["success"].toBool(false);
        QString msg = data["message"].toString();
        emit updateUserResponse(success, msg);
        break;
    }

    // ---------- Room responses (Phase 2) ----------
    case NetworkConstants::MSG_ROOM_CREATED: {
        bool success = data["success"].toBool(false);
        QString msg = data["message"].toString();
        Room room;
        if (success && data.contains("room")) {
            room = NetworkProtocol::roomFromJson(data["room"].toObject());
        } else {
            // Fallback: use pending info to build a room object
            room.roomName = pendingRoomName;
            room.port = pendingPort;
            room.hostUsername = ""; // Will be set by server
            room.gameSettings = pendingSettings;
            room.password = pendingPassword;
            room.status = RoomStatus::Waiting;
        }
        emit roomCreated(success, room, msg);
        break;
    }
    case NetworkConstants::MSG_ROOM_JOINED: {
        bool success = data["success"].toBool(false);
        QString msg = data["message"].toString();
        Room room;
        if (success && data.contains("room")) {
            room = NetworkProtocol::roomFromJson(data["room"].toObject());
        }
        emit roomJoined(success, room, msg);
        break;
    }
    case NetworkConstants::MSG_PLAYER_JOINED: {
        QString playerName = data["playerName"].toString();
        emit playerJoined(playerName);
        break;
    }
    case NetworkConstants::MSG_PLAYER_LEFT: {
        QString playerName = data["playerName"].toString();
        emit playerLeft(playerName);
        break;
    }
    case NetworkConstants::MSG_ROOM_ERROR: {
        QString msg = data["message"].toString();
        emit roomError(msg);
        break;
    }
    case NetworkConstants::MSG_GAME_START:
        emit gameStarted(data);
        break;
    case NetworkConstants::MSG_GAME_STATE:
        emit gameStateReceived(data);
        break;
    case NetworkConstants::MSG_GAME_OVER:
        emit gameOverReceived(data);
        break;
    case NetworkConstants::MSG_GAME_ABORTED:
        emit gameAborted(data["message"].toString());
        break;
    case NetworkConstants::MSG_GAME_ERROR:
        emit gameError(data["message"].toString());
        break;
    default:
        emit error("Unknown response type from server");
        break;
    }
}

void ClientManager::sendGameStart(quint16 port, int boardSize)
{
    if (!isConnected()) {
        emit gameError("Not connected to server");
        return;
    }
    
    QJsonObject data;
    data["port"] = static_cast<int>(port);
    data["boardSize"] = boardSize;
    
    QString message = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_START, data);
    socket->write(message.toUtf8());
}

void ClientManager::sendGameMove(const QJsonObject& moveData)
{
    if (!isConnected()) {
        emit gameError("Not connected to server");
        return;
    }
    
    QString message = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_MOVE, moveData);
    socket->write(message.toUtf8());
}

void ClientManager::sendGameAbort(quint16 port)
{
    if (!isConnected()) return;
    
    QJsonObject data;
    data["port"] = static_cast<int>(port);
    
    QString message = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_ABORTED, data);
    socket->write(message.toUtf8());
}

void ClientManager::sendGameReady(quint16 port, bool ready)
{
    if (!isConnected()) return;
    
    QJsonObject data;
    data["port"] = static_cast<int>(port);
    data["ready"] = ready;
    
    QString message = NetworkProtocol::buildMessage(NetworkConstants::MSG_GAME_READY, data);
    socket->write(message.toUtf8());
}