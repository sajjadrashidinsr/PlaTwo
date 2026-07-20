#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QJsonObject>
#include "GameBoard.h"

class ClientManager;

namespace Ui {
class GameWidget;
}

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    ~GameWidget();

    void setClientManager(ClientManager* client);
    void setRoomInfo(quint16 port, bool isHost);
    void startGame(int boardSize);
    void setPlayers(const QString& player1Name, const QString& player2Name);

public slots:
    void onGameStateReceived(const QJsonObject& data);
    void onGameOverReceived(const QJsonObject& data);
    void onGameAborted();

signals:
    void moveMade(const QPoint& p1, const QPoint& p2);
    void leaveGame();

private slots:
    void onLineClicked(const QPoint& p1, const QPoint& p2);
    void onBackClicked();

private:
    void updateUI();

    Ui::GameWidget *ui;
    GameBoard* m_gameBoard;

    QPointer<ClientManager> m_clientManager;
    quint16 m_roomPort = 0;
    bool m_isHost = false;
    int m_playerId = 0;
    bool m_gameEnded = false;

    int m_player1Score = 0;
    int m_player2Score = 0;
    int m_currentPlayer = 0;
    bool m_gameOver = false;
};

#endif // GAMEWIDGET_H