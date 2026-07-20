#include "GameController.h"
#include "BoardModel.h"
#include "Player.h"
#include <QDebug>

GameController::GameController(int boardSize, QObject* parent)
    : QObject(parent)
    , m_model(std::make_unique<BoardModel>(boardSize, this))
    , m_player1(std::make_unique<Player>("Player 1", QColor(33, 150, 243), this))
    , m_player2(std::make_unique<Player>("Player 2", QColor(244, 67, 54), this))
    , m_currentPlayerId(0)
    , m_gameOver(false)
{
    connect(m_model.get(), &BoardModel::boxCompleted,
            this, &GameController::onBoxCompleted);
}

GameController::~GameController()
{
}

void GameController::setPlayerNames(const QString& player1Name, const QString& player2Name)
{
    m_player1->setName(player1Name);
    m_player2->setName(player2Name);
    emit gameStateChanged();
}

bool GameController::makeMove(const QPoint& p1, const QPoint& p2)
{
    if (m_gameOver) {
        return false;
    }

    if (!m_model->isValidMove(p1, p2)) {
        return false;
    }

    QVector<QPoint> completedBoxes = m_model->makeMove(p1, p2, m_currentPlayerId);

    if (!completedBoxes.isEmpty()) {
        updateScore(m_currentPlayerId, completedBoxes.size());
    } else {
        switchPlayer();
    }

    checkGameOver();
    emit gameStateChanged();
    return true;
}

const Player* GameController::getCurrentPlayer() const
{
    return m_currentPlayerId == 0 ? m_player1.get() : m_player2.get();
}

const Player* GameController::getPlayer(int id) const
{
    if (id == 0) {
        return m_player1.get();
    } else if (id == 1) {
        return m_player2.get();
    }
    return nullptr;
}

bool GameController::isGameOver() const
{
    return m_gameOver;
}

int GameController::getWinner() const
{
    if (!m_gameOver) {
        return -1;
    }

    int score1 = m_player1->getScore();
    int score2 = m_player2->getScore();

    if (score1 > score2) {
        return 0;
    } else if (score2 > score1) {
        return 1;
    } else {
        return -1;
    }
}

void GameController::resetGame()
{
    m_model->reset();
    m_player1->resetScore();
    m_player2->resetScore();
    m_currentPlayerId = 0;
    m_gameOver = false;
    emit gameStateChanged();
}

void GameController::newGame(int boardSize)
{
    m_model = std::make_unique<BoardModel>(boardSize, this);
    connect(m_model.get(), &BoardModel::boxCompleted,
            this, &GameController::onBoxCompleted);

    m_player1->resetScore();
    m_player2->resetScore();
    m_currentPlayerId = 0;
    m_gameOver = false;
    emit gameStateChanged();
}

void GameController::switchPlayer()
{
    m_currentPlayerId = (m_currentPlayerId == 0) ? 1 : 0;
}

void GameController::onBoxCompleted(int row, int col, int playerId)
{
    Q_UNUSED(row);
    Q_UNUSED(col);
    Q_UNUSED(playerId);
}

void GameController::checkGameOver()
{
    if (m_model->isGameOver()) {
        m_gameOver = true;
        emit gameStateChanged();
    }
}

void GameController::updateScore(int playerId, int boxesCompleted)
{
    Player* player = (playerId == 0) ? m_player1.get() : m_player2.get();
    if (player) {
        player->addScore(boxesCompleted);
    }
}