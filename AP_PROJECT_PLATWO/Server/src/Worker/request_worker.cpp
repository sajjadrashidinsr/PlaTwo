#include "request_worker.h"
#include <QMetaObject>
#include <QDebug>
#include <QThread>

RequestWorker::RequestWorker(QTcpSocket* clientSocket,
                             const QString& message,
                             storage_manager* storage,
                             RoomManager* roomManager,
                             QObject* parent)
    : QRunnable()
    , socket(clientSocket)
    , messageBuffer(message)
    , storageManager(storage)
    , roomManager(roomManager)
    , parentObject(parent) {

    setAutoDelete(true);
    qDebug() << "[Worker] Created, thread ID:" << QThread::currentThreadId();
}

RequestWorker::~RequestWorker() {
    qDebug() << "[Worker] Destroyed";
}

void RequestWorker::run() {
    qDebug() << "[Worker] Running on thread ID:" << QThread::currentThreadId();
    qDebug() << "[Worker] Processing message:" << messageBuffer;

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
    // Authentication
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
    // Room management
    case NetworkConstants::MSG_CREATE_ROOM:
        handleCreateRoom(data);
        break;
    case NetworkConstants::MSG_JOIN_ROOM:
        handleJoinRoom(data);
        break;
    case NetworkConstants::MSG_LEAVE_ROOM:
        handleLeaveRoom(data);
        break;
    default:
        sendResponse(NetworkConstants::MSG_ERROR, false, "Unknown message type");
        break;
    }
}

void RequestWorker::sendResponse(int type, bool success,
                                 const QString& message, const QJsonObject& data) {
    QJsonObject responseData = data;
    responseData["success"] = success;
    responseData["message"] = message;

    QString response = NetworkProtocol::buildMessage(type, responseData);

    qDebug() << "[Worker] Sending response - Success:" << success << "Message:" << message;

    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        socket->write(response.toUtf8());
        socket->flush();
        qDebug() << "[Worker] Response sent";
    } else {
        qDebug() << "[Worker] Socket not connected!";
    }
}

// ---------- Authentication Handlers ----------
void RequestWorker::handleRegister(const QJsonObject& data) {
    qDebug() << "[Worker] handleRegister called";

    if (!data.contains("username") || !data.contains("passwordHash") ||
        !data.contains("name") || !data.contains("phone") || !data.contains("email")) {
        sendResponse(NetworkConstants::MSG_REGISTER, false,
                     "Missing required fields");
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
        sendResponse(NetworkConstants::MSG_REGISTER, false,
                     "Password must be at least 8 characters");
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
        sendResponse(NetworkConstants::MSG_REGISTER, false,
                     "Username already exists or database error");
    }
}

void RequestWorker::handleLogin(const QJsonObject& data) {
    qDebug() << "[Worker] handleLogin called";

    QString username = data["username"].toString();
    QString password = data["password"].toString();

    if (username.isEmpty() || password.isEmpty()) {
        sendResponse(NetworkConstants::MSG_LOGIN, false,
                     "Username and password required");
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
        // ✅ Store username in socket property for future requests
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
        sendResponse(NetworkConstants::MSG_FORGOT_PASSWORD, false,
                     "Username and phone required");
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
        sendResponse(NetworkConstants::MSG_CHANGE_PASSWORD, false,
                     "Password must be at least 8 characters");
        return;
    }

    user* u = storageManager->getuser(username);
    if (!u) {
        sendResponse(NetworkConstants::MSG_CHANGE_PASSWORD, false, "User not found");
        return;
    }

    if (!AuthManager::verifyPhoneForRecovery(u->phone, phone)) {
        sendResponse(NetworkConstants::MSG_CHANGE_PASSWORD, false,
                     "Phone number does not match");
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

// ---------- Room Handlers ----------
void RequestWorker::handleCreateRoom(const QJsonObject& data) {
    qDebug() << "[Worker] handleCreateRoom called";

    // Retrieve username from socket property
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

        // Notify the host about the guest joining
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

    // Expect the client to send the port of the room to leave
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