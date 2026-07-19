/**
 * @file GameController.cpp
 * @brief Implementation of the GameController class.
 */

#include "GameController.h"
#include "BoardModel.h"
#include "Player.h"

#include <QDebug>
#include <QVector>

/**
 * @brief Constructor for GameController.
 * @param boardSize The size of the board.
 * @param parent Parent object.
 */
GameController::GameController(int boardSize, QObject* parent)
    : QObject(parent)
    , m_model(std::make_unique<BoardModel>(boardSize, this))
    , m_player1(std::make_unique<Player>("Player 1", QColor(33, 150, 243), this))  // Blue
    , m_player2(std::make_unique<Player>("Player 2", QColor(244, 67, 54), this))   // Red
    , m_currentPlayerId(0)
    , m_gameOver(false)
{
    // Connect signals
    connect(m_model.get(), &BoardModel::boxCompleted,
            this, &GameController::onBoxCompleted);
}

/**
 * @brief Destructor for GameController.
 */
GameController::~GameController()
{
    // unique_ptrs handle cleanup
}

/**
 * @brief Sets the names of the players.
 * @param player1Name Name for player 1.
 * @param player2Name Name for player 2.
 */
void GameController::setPlayerNames(const QString& player1Name, const QString& player2Name)
{
    m_player1->setName(player1Name);
    m_player2->setName(player2Name);
    emit gameStateChanged();
}

/**
 * @brief Makes a move on the board.
 * @param p1 First endpoint.
 * @param p2 Second endpoint.
 * @return True if the move was valid.
 */
bool GameController::makeMove(const QPoint& p1, const QPoint& p2)
{
    if (m_gameOver) {
        return false;
    }
    
    // Validate the move
    if (!m_model->isValidMove(p1, p2)) {
        return false;
    }
    
    // Make the move
    QVector<QPoint> completedBoxes = m_model->makeMove(p1, p2, m_currentPlayerId);
    
    // Update score if boxes were completed
    if (!completedBoxes.isEmpty()) {
        updateScore(m_currentPlayerId, completedBoxes.size());
        // Player gets another turn (don't switch players)
    } else {
        // Switch to the other player
        switchPlayer();
    }
    
    // Check if the game is over
    checkGameOver();
    
    emit gameStateChanged();
    return true;
}

/**
 * @brief Gets the current player.
 * @return Pointer to the current player.
 */
const Player* GameController::getCurrentPlayer() const
{
    return m_currentPlayerId == 0 ? m_player1.get() : m_player2.get();
}

/**
 * @brief Gets a player by ID.
 * @param id The player ID.
 * @return Pointer to the player.
 */
const Player* GameController::getPlayer(int id) const
{
    if (id == 0) {
        return m_player1.get();
    } else if (id == 1) {
        return m_player2.get();
    }
    return nullptr;
}

/**
 * @brief Checks if the game is over.
 * @return True if game over.
 */
bool GameController::isGameOver() const
{
    return m_gameOver;
}

/**
 * @brief Gets the winner of the game.
 * @return Player ID (0 or 1) or -1 for draw.
 */
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
        return -1;  // Draw
    }
}

/**
 * @brief Resets the game with the same board size.
 */
void GameController::resetGame()
{
    m_model->reset();
    m_player1->resetScore();
    m_player2->resetScore();
    m_currentPlayerId = 0;
    m_gameOver = false;
    emit gameStateChanged();
}

/**
 * @brief Starts a new game with a new board size.
 * @param boardSize The new board size.
 */
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

/**
 * @brief Slot called when a box is completed.
 * @param row The row of the box.
 * @param col The column of the box.
 * @param playerId The ID of the player.
 */
void GameController::onBoxCompleted(int row, int col, int playerId)
{
    // Score is updated in makeMove()
    Q_UNUSED(row);
    Q_UNUSED(col);
    Q_UNUSED(playerId);
}

/**
 * @brief Switches to the next player.
 */
void GameController::switchPlayer()
{
    m_currentPlayerId = (m_currentPlayerId == 0) ? 1 : 0;
}

/**
 * @brief Checks if the game is over.
 */
void GameController::checkGameOver()
{
    if (m_model->isGameOver()) {
        m_gameOver = true;
        emit gameStateChanged();
    }
}

/**
 * @brief Updates the score for a player.
 * @param playerId The player ID.
 * @param boxesCompleted The number of boxes completed.
 */
void GameController::updateScore(int playerId, int boxesCompleted)
{
    Player* player = (playerId == 0) ? m_player1.get() : m_player2.get();
    if (player) {
        player->addScore(boxesCompleted);
    }
}