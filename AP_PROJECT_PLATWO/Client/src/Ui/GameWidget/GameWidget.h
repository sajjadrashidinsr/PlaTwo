#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QPointer>
#include <QJsonObject>
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QResizeEvent>
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

protected:
    void resizeEvent(QResizeEvent* event) override;

public slots:
    void onGameStateReceived(const QJsonObject& data);
    void onGameOverReceived(const QJsonObject& data);
    void onGameAborted();

signals:
    void leaveGame();

private slots:
    void onLineClicked(const QPoint& p1, const QPoint& p2);
    void onBackClicked();
    void onReturnToMenuClicked();

private:
    void updateUI();
    void setupGameOverOverlay();
    void showGameOverDialog(const QString& winnerName, int winnerId);
    void hideGameOverDialog();

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

    QString m_player1Name;
    QString m_player2Name;

    QWidget* m_gameOverOverlay = nullptr;
    QLabel* m_gameOverTitle = nullptr;
    QLabel* m_gameOverMessage = nullptr;
    QPushButton* m_returnToMenuButton = nullptr;
};

#endif // GAMEWIDGET_H