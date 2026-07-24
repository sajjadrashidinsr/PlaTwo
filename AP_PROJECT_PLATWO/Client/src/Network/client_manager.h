#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <memory>  // ← ADD THIS
#include "network_constants.h"
#include "network_protocol.h"
#include "user.h"
#include "../Models/room.h"

class ClientManager : public QObject {
    Q_OBJECT

public:
    explicit ClientManager(QObject* parent = nullptr);
    ~ClientManager();

    bool connectToServer(const QString& address = NetworkConstants::SERVER_ADDRESS,
                         quint16 port = NetworkConstants::DEFAULT_PORT);
    void disconnectFromServer();
    bool isConnected() const { return socket && socket->state() == QAbstractSocket::ConnectedState; }

    // Authentication
    void sendRegister(const user& newUser);
    void sendLogin(const QString& username, const QString& password);
    void sendForgotPassword(const QString& username, const QString& phone);
    void sendChangePassword(const QString& username, const QString& phone,
                            const QString& newPassword);
    void sendGetUser(const QString& username);
    void sendUpdateUser(const QString& oldUsername, const user& updatedUser);

    // Room Management
    void createRoom(const QString& roomName, quint16 port,
                    const GameSettings& settings, const QString& password = QString());
    void joinRoom(const QString& ip, quint16 port, const QString& password = QString());
    void leaveRoom(quint16 port);

    void sendGameStart(quint16 port, int boardSize);
    void sendGameMove(const QJsonObject& moveData);
    void sendGameAbort(quint16 port);
    void sendGameReady(quint16 port, bool ready);

signals:
    // Connection
    void connected();
    void disconnected();
    void error(const QString& errorMessage);

    // Authentication responses (CHANGED: user* → std::shared_ptr<user>)
    void registerResponse(bool success, const QString& message);
    void loginResponse(bool success, std::shared_ptr<user> userData, const QString& message);
    void forgotPasswordResponse(bool success, const QString& message);
    void passwordChangedResponse(bool success, const QString& message);
    void getUserResponse(bool success, std::shared_ptr<user> userData, const QString& message);
    void updateUserResponse(bool success, const QString& message);

    // Room responses
    void roomCreated(bool success, const Room& room, const QString& message);
    void roomJoined(bool success, const Room& room, const QString& message);
    void playerJoined(const QString& playerName);
    void playerLeft(const QString& playerName);
    void roomError(const QString& error);

    void gameStarted(const QJsonObject& data);
    void gameStateReceived(const QJsonObject& state);
    void gameOverReceived(const QJsonObject& data);
    void gameAborted(const QString& message);
    void gameError(const QString& error);

private slots:
    void onReadyRead();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError socketError);
    void onConnectionTimeout();

private:
    void processMessage(const QString& message);

    QTcpSocket* socket;
    QTimer* connectionTimer;
    QString buffer;
    bool awaitingResponse;

    // Temporary room state
    QString pendingRoomName;
    quint16 pendingPort;
    GameSettings pendingSettings;
    QString pendingPassword;
};

#endif // CLIENT_MANAGER_H