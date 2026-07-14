#include "client_manager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

ClientManager::ClientManager(QObject* parent)
    : QObject(parent)
    , socket(nullptr)
    , connectionTimer(nullptr)
    , awaitingResponse(false) {

    socket = new QTcpSocket(this);

    connectionTimer = new QTimer(this);
    connectionTimer->setSingleShot(true);
    connectionTimer->setInterval(3000);

    // اتصال سیگنال‌ها با روش صحیح برای Qt 6
    connect(socket, &QTcpSocket::connected, this, &ClientManager::onConnected);
    connect(socket, &QTcpSocket::disconnected, this, &ClientManager::onDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &ClientManager::onReadyRead);

    // ✅ استفاده از errorOccurred در Qt 6
    connect(socket, &QTcpSocket::errorOccurred,
            this, &ClientManager::onError);

    connect(connectionTimer, &QTimer::timeout, this, &ClientManager::onConnectionTimeout);
}

ClientManager::~ClientManager() {
    disconnectFromServer();
}

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

void ClientManager::sendUpdateUser(const user& updatedUser) {
    if (!isConnected()) {
        emit error("Not connected to server");
        return;
    }

    QJsonObject userObj = NetworkProtocol::userToJson(updatedUser);
    QString message = NetworkProtocol::buildMessage(
        NetworkConstants::MSG_UPDATE_USER,
        userObj
        );

    socket->write(message.toUtf8());
    awaitingResponse = true;
}

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
    default:
        emit error("Unknown response type from server");
        break;
    }
}