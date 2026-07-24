#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include <memory>
#include <QPoint>
#include "BoardModel.h"
#include "Player.h"


class GameController : public QObject
{
    Q_OBJECT

public:
    explicit GameController(int boardSize, QObject* parent = nullptr);
    ~GameController() override;

    void setPlayerNames(const QString& player1Name, const QString& player2Name);
    bool makeMove(const QPoint& p1, const QPoint& p2);
    const Player* getCurrentPlayer() const;
    const Player* getPlayer(int id) const;
    const BoardModel* getModel() const { return m_model.get(); }
    bool isGameOver() const;
    int getWinner() const;
    void resetGame();
    void newGame(int boardSize);
    void switchPlayer();
    void setPlayerId(int playerIndex, int id) {
        Player* player = (playerIndex == 0) ? m_player1.get() : m_player2.get();
        if (player) {
            player->setPlayerId(id);
        }
    }

signals:
    void gameStateChanged();

private slots:
    void onBoxCompleted(int row, int col, int playerId);

private:
    void checkGameOver();
    void updateScore(int playerId, int boxesCompleted);

    std::unique_ptr<BoardModel> m_model;
    std::unique_ptr<Player> m_player1;
    std::unique_ptr<Player> m_player2;
    int m_currentPlayerId;
    bool m_gameOver;
};

#endif // GAMECONTROLLER_H