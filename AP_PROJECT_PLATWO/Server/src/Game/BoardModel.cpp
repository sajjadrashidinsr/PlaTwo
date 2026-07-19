/**
 * @file BoardModel.cpp
 * @brief Implementation of the BoardModel class.
 */

#include "BoardModel.h"
#include <QPoint>
#include <QDebug>
#include <algorithm>

/**
 * @brief Constructor for BoardModel.
 */
BoardModel::BoardModel(int boardSize, QObject* parent)
    : QObject(parent)
    , m_boardSize(boardSize)
{
    reset();
}

/**
 * @brief Destructor for BoardModel.
 */
BoardModel::~BoardModel()
{
    // Qt handles deletion
}

/**
 * @brief Gets the owner of a line.
 */
int BoardModel::getLine(int row, int col, LineType type) const
{
    if (row < 0 || row >= m_boardSize || col < 0 || col >= m_boardSize) {
        return INVALID_PLAYER;
    }

    if (type == LineType::Horizontal) {
        if (col >= m_boardSize - 1) return INVALID_PLAYER;
        return m_horizontalLines[row][col];
    } else {  // Vertical
        if (row >= m_boardSize - 1) return INVALID_PLAYER;
        return m_verticalLines[row][col];
    }
}

/**
 * @brief Sets a line to be drawn by a player.
 */
bool BoardModel::setLine(int row, int col, LineType type, int playerId)
{
    if (row < 0 || row >= m_boardSize || col < 0 || col >= m_boardSize) {
        return false;
    }

    if (playerId < 0 || playerId > 1) {
        return false;
    }

    if (type == LineType::Horizontal) {
        if (col >= m_boardSize - 1) return false;
        if (m_horizontalLines[row][col] != INVALID_PLAYER) {
            return false;
        }
        m_horizontalLines[row][col] = playerId;
        return true;
    } else {  // Vertical
        if (row >= m_boardSize - 1) return false;
        if (m_verticalLines[row][col] != INVALID_PLAYER) {
            return false;
        }
        m_verticalLines[row][col] = playerId;
        return true;
    }
}

/**
 * @brief Gets the owner of a box.
 */
int BoardModel::getBox(int row, int col) const
{
    if (row < 0 || row >= m_boardSize - 1 || col < 0 || col >= m_boardSize - 1) {
        return INVALID_PLAYER;
    }
    return m_boxes[row][col];
}

/**
 * @brief Checks if a line is already drawn.
 */
bool BoardModel::isLineDrawn(int row, int col, LineType type) const
{
    return getLine(row, col, type) != INVALID_PLAYER;
}

/**
 * @brief Checks if a move is valid.
 */
bool BoardModel::isValidMove(const QPoint& p1, const QPoint& p2) const
{
    int row, col;
    LineType type;

    if (!getLineIndices(p1, p2, row, col, type)) {
        return false;
    }

    return getLine(row, col, type) == INVALID_PLAYER;
}

/**
 * @brief Makes a move on the board.
 */
QVector<QPoint> BoardModel::makeMove(const QPoint& p1, const QPoint& p2, int playerId)
{
    QVector<QPoint> completedBoxes;

    int row, col;
    LineType type;

    if (!getLineIndices(p1, p2, row, col, type)) {
        return completedBoxes;
    }

    if (!setLine(row, col, type, playerId)) {
        return completedBoxes;
    }

    if (type == LineType::Horizontal) {
        if (row > 0 && isBoxComplete(row - 1, col)) {
            m_boxes[row - 1][col] = playerId;
            completedBoxes.append(QPoint(row - 1, col));
            emit boxCompleted(row - 1, col, playerId);
        }
        if (row < m_boardSize - 1 && isBoxComplete(row, col)) {
            m_boxes[row][col] = playerId;
            completedBoxes.append(QPoint(row, col));
            emit boxCompleted(row, col, playerId);
        }
    } else {
        if (col > 0 && isBoxComplete(row, col - 1)) {
            m_boxes[row][col - 1] = playerId;
            completedBoxes.append(QPoint(row, col - 1));
            emit boxCompleted(row, col - 1, playerId);
        }
        if (col < m_boardSize - 1 && isBoxComplete(row, col)) {
            m_boxes[row][col] = playerId;
            completedBoxes.append(QPoint(row, col));
            emit boxCompleted(row, col, playerId);
        }
    }

    return completedBoxes;
}

/**
 * @brief Checks if the game is over.
 */
bool BoardModel::isGameOver() const
{
    for (int row = 0; row < m_boardSize - 1; ++row) {
        for (int col = 0; col < m_boardSize - 1; ++col) {
            if (m_boxes[row][col] == INVALID_PLAYER) {
                return false;
            }
        }
    }
    return true;
}

/**
 * @brief Gets the total number of boxes.
 */
int BoardModel::getTotalBoxes() const
{
    return (m_boardSize - 1) * (m_boardSize - 1);
}

/**
 * @brief Gets the number of boxes claimed by a player.
 */
int BoardModel::getBoxesForPlayer(int playerId) const
{
    int count = 0;
    for (int row = 0; row < m_boardSize - 1; ++row) {
        for (int col = 0; col < m_boardSize - 1; ++col) {
            if (m_boxes[row][col] == playerId) {
                ++count;
            }
        }
    }
    return count;
}

/**
 * @brief Resets the board.
 */
void BoardModel::reset()
{
    m_horizontalLines.resize(m_boardSize);
    for (int row = 0; row < m_boardSize; ++row) {
        m_horizontalLines[row].fill(INVALID_PLAYER, m_boardSize - 1);
    }

    m_verticalLines.resize(m_boardSize - 1);
    for (int row = 0; row < m_boardSize - 1; ++row) {
        m_verticalLines[row].fill(INVALID_PLAYER, m_boardSize);
    }

    m_boxes.resize(m_boardSize - 1);
    for (int row = 0; row < m_boardSize - 1; ++row) {
        m_boxes[row].fill(INVALID_PLAYER, m_boardSize - 1);
    }
}

/**
 * @brief Checks if a box is completed.
 */
bool BoardModel::isBoxComplete(int row, int col) const
{
    if (row < 0 || row >= m_boardSize - 1 || col < 0 || col >= m_boardSize - 1) {
        return false;
    }

    if (m_boxes[row][col] != INVALID_PLAYER) {
        return false;
    }

    return m_horizontalLines[row][col] != INVALID_PLAYER &&
           m_horizontalLines[row + 1][col] != INVALID_PLAYER &&
           m_verticalLines[row][col] != INVALID_PLAYER &&
           m_verticalLines[row][col + 1] != INVALID_PLAYER;
}

/**
 * @brief Checks if the four lines of a box are drawn.
 */
bool BoardModel::areBoxLinesDrawn(int row, int col) const
{
    if (row < 0 || row >= m_boardSize - 1 || col < 0 || col >= m_boardSize - 1) {
        return false;
    }

    return m_horizontalLines[row][col] != INVALID_PLAYER &&
           m_horizontalLines[row + 1][col] != INVALID_PLAYER &&
           m_verticalLines[row][col] != INVALID_PLAYER &&
           m_verticalLines[row][col + 1] != INVALID_PLAYER;
}

/**
 * @brief Converts line endpoints to grid indices.
 */
bool BoardModel::getLineIndices(const QPoint& p1, const QPoint& p2,
                                int& row, int& col, LineType& type) const
{
    if (p1.x() < 0 || p1.x() >= m_boardSize || p1.y() < 0 || p1.y() >= m_boardSize ||
        p2.x() < 0 || p2.x() >= m_boardSize || p2.y() < 0 || p2.y() >= m_boardSize) {
        return false;
    }

    int dx = qAbs(p1.x() - p2.x());
    int dy = qAbs(p1.y() - p2.y());

    if ((dx == 1 && dy == 0) || (dx == 0 && dy == 1)) {
        if (dx == 1) {
            type = LineType::Vertical;
            row = qMin(p1.x(), p2.x());
            col = p1.y();
        } else {
            type = LineType::Horizontal;
            row = p1.x();
            col = qMin(p1.y(), p2.y());
        }
        return true;
    }

    return false;
}