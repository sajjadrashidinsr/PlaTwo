#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include "network_constants.h"
#include "network_protocol.h"
#include "user.h"

class ClientManager : public QObject {
    Q_OBJECT

public:
    explicit ClientManager(QObject* parent = nullptr);
    ~ClientManager();

    bool connectToServer(const QString& address = NetworkConstants::SERVER_ADDRESS,
                         quint16 port = NetworkConstants::DEFAULT_PORT);
    void disconnectFromServer();
    bool isConnected() const { return socket && socket->state() == QAbstractSocket::ConnectedState; }

    void sendRegister(const user& newUser);
    void sendLogin(const QString& username, const QString& password);
    void sendForgotPassword(const QString& username, const QString& phone);
    void sendChangePassword(const QString& username, const QString& phone,
                            const QString& newPassword);
    void sendGetUser(const QString& username);

    // ✅ تغییر مهم: اضافه شدن oldUsername به ورودی تابع
    void sendUpdateUser(const QString& oldUsername, const user& updatedUser);

signals:
    void connected();
    void disconnected();
    void error(const QString& errorMessage);

    void registerResponse(bool success, const QString& message);
    void loginResponse(bool success, user* userData, const QString& message);
    void forgotPasswordResponse(bool success, const QString& message);
    void passwordChangedResponse(bool success, const QString& message);
    void getUserResponse(bool success, user* userData, const QString& message);
    void updateUserResponse(bool success, const QString& message);

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
};

#endif // CLIENT_MANAGER_H