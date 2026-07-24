#ifndef REQUEST_WORKER_H
#define REQUEST_WORKER_H

#include <QRunnable>
#include <QTcpSocket>
#include <QJsonObject>
#include <QString>
#include <QPointer>
#include <memory>
#include "network_constants.h"
#include "network_protocol.h"
#include "storage_manager.h"
#include "auth_manager.h"
#include "user.h"
#include "../Server/roommanager.h"

class RequestWorker : public QRunnable {
public:
    RequestWorker(QTcpSocket* clientSocket,
                  const QString& message,
                  std::shared_ptr<storage_manager> storage,
                  std::shared_ptr<RoomManager> roomManager,
                  QObject* parent = nullptr);

    ~RequestWorker();

    void run() override;

private:
    void processMessage(const QString& message);
    void sendResponse(int type, bool success, const QString& message,
                      const QJsonObject& data = QJsonObject());

    // Authentication handlers
    void handleRegister(const QJsonObject& data);
    void handleLogin(const QJsonObject& data);
    void handleForgotPassword(const QJsonObject& data);
    void handleChangePassword(const QJsonObject& data);
    void handleGetUser(const QJsonObject& data);
    void handleUpdateUser(const QJsonObject& data);

    // Room handlers
    void handleCreateRoom(const QJsonObject& data);
    void handleJoinRoom(const QJsonObject& data);
    void handleLeaveRoom(const QJsonObject& data);
    void handleGameStart(const QJsonObject& data);
    void handleGameMove(const QJsonObject& data);
    void handleGameAbort(const QJsonObject& data);
    void handleGameReady(const QJsonObject& data);

    QPointer<QTcpSocket> socket;                              // ✅
    QString messageBuffer;
    std::shared_ptr<storage_manager> storageManager;         // ✅
    std::shared_ptr<RoomManager> roomManager;                // ✅
    QPointer<QObject> parentObject;                          // ✅
};

#endif // REQUEST_WORKER_H