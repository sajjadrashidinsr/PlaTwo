#include "threaded_server.h"
#include "request_worker.h"
#include <QDebug>

ThreadedServer::ThreadedServer(QObject* parent)
    : QObject(parent)
    , tcpServer(nullptr)
    , storageManager(nullptr)
    , roomManager(nullptr)
    , threadPool(nullptr) {

    storageManager = new storage_manager();
    roomManager = new RoomManager(this);

    tcpServer = new QTcpServer(this);

    threadPool = new QThreadPool(this);
    threadPool->setMaxThreadCount(10);
    threadPool->setExpiryTimeout(30000);

    connect(tcpServer, &QTcpServer::newConnection,
            this, &ThreadedServer::onNewConnection);

    qDebug() << "[Server] ThreadPool created with max threads:" << threadPool->maxThreadCount();
    qDebug() << "[Server] Storage manager initialized";
    qDebug() << "[Server] RoomManager initialized";
}

ThreadedServer::~ThreadedServer() {
    stopServer();
    if (storageManager) {
        delete storageManager;
        storageManager = nullptr;
    }
    qDebug() << "[Server] Server destroyed";
}

bool ThreadedServer::startServer(quint16 port) {
    if (tcpServer->listen(QHostAddress::Any, port)) {
        qDebug() << "[Server] ========================================";
        qDebug() << "[Server] ThreadedServer started successfully";
        qDebug() << "[Server] Port:" << port;
        qDebug() << "[Server] Thread pool max threads:" << threadPool->maxThreadCount();
        qDebug() << "[Server] Database will be created if not exists";
        qDebug() << "[Server] Existing data will be preserved";
        qDebug() << "[Server] Waiting for clients...";
        qDebug() << "[Server] ========================================";
        return true;
    } else {
        qDebug() << "[Server] Failed to start server:" << tcpServer->errorString();
        return false;
    }
}

void ThreadedServer::stopServer() {
    qDebug() << "[Server] Stopping server...";

    if (tcpServer) {
        tcpServer->close();
    }

    if (threadPool) {
        threadPool->waitForDone(5000);
        qDebug() << "[Server] All threads finished";
    }

    QMutexLocker locker(&clientMutex);
    for (auto it = clientBuffer.begin(); it != clientBuffer.end(); ++it) {
        if (it.key()) {
            it.key()->disconnectFromHost();
            it.key()->deleteLater();
        }
    }
    clientBuffer.clear();
    clientUsername.clear();
    clientPort.clear();

    qDebug() << "[Server] Server stopped";
}

void ThreadedServer::onNewConnection() {
    QTcpSocket* client = tcpServer->nextPendingConnection();

    if (!client) {
        return;
    }

    connect(client, &QTcpSocket::readyRead, this, &ThreadedServer::onReadyRead);
    connect(client, &QTcpSocket::disconnected, this, &ThreadedServer::onClientDisconnected);

    QMutexLocker locker(&clientMutex);
    clientBuffer[client] = "";
    clientUsername[client] = "";
    clientPort[client] = client->peerPort();

    QString clientAddress = client->peerAddress().toString();
    quint16 clientPortNum = client->peerPort();

    emit clientConnected(clientAddress, clientPortNum);

    qDebug() << "[Main Thread] New client connected:"
             << clientAddress
             << "port:" << clientPortNum
             << "Active threads:" << threadPool->activeThreadCount();
}

void ThreadedServer::onClientDisconnected() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        QString clientAddress = client->peerAddress().toString();
        quint16 clientPortNum = client->peerPort();

        emit clientDisconnected(clientAddress, clientPortNum);

        qDebug() << "[Main Thread] Client disconnected:"
                 << clientAddress
                 << "port:" << clientPortNum;

        cleanupClient(client);
    }
}

void ThreadedServer::onReadyRead() {
    QTcpSocket* client = qobject_cast<QTcpSocket*>(sender());
    if (!client) {
        return;
    }

    QByteArray data = client->readAll();

    QMutexLocker locker(&clientMutex);
    clientBuffer[client] += QString::fromUtf8(data);

    while (clientBuffer[client].contains('\n')) {
        int separatorPos = clientBuffer[client].indexOf('\n');
        QString message = clientBuffer[client].left(separatorPos);
        clientBuffer[client].remove(0, separatorPos + 1);

        if (!message.isEmpty()) {
            qDebug() << "[Main Thread] Received message from client, length:" << message.length();

            RequestWorker* worker = new RequestWorker(
                client,
                message,
                storageManager,
                roomManager,
                this
                );

            threadPool->start(worker);

            QString clientAddress = client->peerAddress().toString();

            qDebug() << "[Main Thread] Task submitted to thread pool."
                     << "Client:" << clientAddress
                     << "Active threads:" << threadPool->activeThreadCount();
        }
    }
}

void ThreadedServer::cleanupClient(QTcpSocket* client) {
    QMutexLocker locker(&clientMutex);
    clientBuffer.remove(client);
    clientUsername.remove(client);
    clientPort.remove(client);
    if (client) {
        client->deleteLater();
    }
}