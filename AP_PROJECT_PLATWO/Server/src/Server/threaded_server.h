#ifndef THREADED_SERVER_H
#define THREADED_SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QThreadPool>
#include <QMutex>
#include "storage_manager.h"
#include "network_constants.h"


class RequestWorker;

class ThreadedServer : public QObject {
    Q_OBJECT

public:
    explicit ThreadedServer(QObject* parent = nullptr);
    ~ThreadedServer();

    bool startServer(quint16 port = NetworkConstants::DEFAULT_PORT);
    void stopServer();
    bool isRunning() const { return tcpServer && tcpServer->isListening(); }
    int activeThreadCount() const { return threadPool ? threadPool->activeThreadCount() : 0; }

signals:
    void clientConnected(const QString& address, quint16 port);
    void clientDisconnected(const QString& address, quint16 port);

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onReadyRead();

private:
    void cleanupClient(QTcpSocket* client);

    QTcpServer* tcpServer;
    storage_manager* storageManager;
    QThreadPool* threadPool;

    QMap<QTcpSocket*, QString> clientBuffer;
    QMap<QTcpSocket*, QString> clientUsername;
    QMap<QTcpSocket*, quint16> clientPort;

    QMutex clientMutex;
};

#endif // THREADED_SERVER_H