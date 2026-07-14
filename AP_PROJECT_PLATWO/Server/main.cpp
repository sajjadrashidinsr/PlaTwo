#include <QCoreApplication>
#include <QDebug>
#include "src/Server/threaded_server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ThreadedServer server;
    if (server.startServer(1234)) {
        qDebug() << "========================================";
        qDebug() << "Server started successfully on port 1234";
        qDebug() << "Database will be created if it doesn't exist";
        qDebug() << "Existing data will be preserved";
        qDebug() << "Waiting for clients...";
        qDebug() << "========================================";
    } else {
        qDebug() << "Failed to start server";
        return 1;
    }

    return a.exec();
}