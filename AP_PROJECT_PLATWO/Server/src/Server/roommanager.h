#ifndef ROOMMANAGER_H
#define ROOMMANAGER_H

#include <QObject>
#include <QMutex>
#include <QMap>
#include "../Models/room.h"

class QTcpSocket;

class RoomManager : public QObject
{
    Q_OBJECT
public:
    explicit RoomManager(QObject *parent = nullptr);
    ~RoomManager();

    bool createRoom(const QString& roomName, quint16 port,
                    const QString& hostUsername,
                    QTcpSocket* hostSocket,
                    const GameSettings& settings,
                    const QString& password = QString(),
                    Room* outRoom = nullptr);

    bool joinRoom(quint16 port, const QString& guestUsername,
                  QTcpSocket* guestSocket,
                  const QString& password = QString(),
                  Room* outRoom = nullptr);

    bool leaveRoom(quint16 port, const QString& username);
    bool removeRoom(quint16 port);
    Room* getRoom(quint16 port);

    QList<Room*> getAllRooms() const;

    bool startGame(quint16 port, int boardSize);
    bool processGameMove(quint16 port, const QPoint& p1, const QPoint& p2,
                         int playerId, QVector<QPoint>& completedBoxes);
    bool endGame(quint16 port, int& winner, int& p1Score, int& p2Score);
    QJsonObject getGameState(quint16 port);

signals:
    void roomCreated(quint16 port);
    void roomJoined(quint16 port, const QString& guestUsername);
    void playerLeft(quint16 port, const QString& username);
    void roomClosed(quint16 port);

private:
    void cleanupRooms();

    QMap<quint16, Room*> rooms;
    mutable QMutex mutex;
};

#endif // ROOMMANAGER_H