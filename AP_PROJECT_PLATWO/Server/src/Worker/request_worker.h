#ifndef REQUEST_WORKER_H
#define REQUEST_WORKER_H

#include <QRunnable>
#include <QTcpSocket>
#include <QJsonObject>
#include <QString>
#include "network_constants.h"
#include "network_protocol.h"
#include "storage_manager.h"
#include "auth_manager.h"
#include "user.h"

class RequestWorker : public QRunnable {
public:
    RequestWorker(QTcpSocket* clientSocket,
                  const QString& message,
                  storage_manager* storage,
                  QObject* parent = nullptr);

    ~RequestWorker();

    void run() override;

private:
    void processMessage(const QString& message);
    void sendResponse(int type, bool success, const QString& message,
                      const QJsonObject& data = QJsonObject());

    void handleRegister(const QJsonObject& data);
    void handleLogin(const QJsonObject& data);
    void handleForgotPassword(const QJsonObject& data);
    void handleChangePassword(const QJsonObject& data);
    void handleGetUser(const QJsonObject& data);
    void handleUpdateUser(const QJsonObject& data);

    QTcpSocket* socket;
    QString messageBuffer;
    storage_manager* storageManager;
    QObject* parentObject;
    QString clientUsername;
};

#endif // REQUEST_WORKER_H