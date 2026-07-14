#include "request_worker.h"
#include <QMetaObject>
#include <QDebug>
#include <QThread>

RequestWorker::RequestWorker(QTcpSocket* clientSocket,
                             const QString& message,
                             storage_manager* storage,
                             QObject* parent)
    : QRunnable()
    , socket(clientSocket)
    , messageBuffer(message)
    , storageManager(storage)
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

    qDebug() << "[Worker] Registering user:" << newUser.username;

    // اعتبارسنجی
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
        clientUsername = username;

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
    if (!data.contains("username")) {
        sendResponse(NetworkConstants::MSG_UPDATE_USER, false, "Username required");
        return;
    }

    user updatedUser = NetworkProtocol::userFromJson(data);

    if (storageManager->updateuser(updatedUser)) {
        sendResponse(NetworkConstants::MSG_UPDATE_USER, true, "User updated successfully");
    } else {
        sendResponse(NetworkConstants::MSG_UPDATE_USER, false, "Failed to update user");
    }
}