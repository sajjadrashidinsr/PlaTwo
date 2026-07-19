/**
 * @file Player.h
 * @brief Declaration of the Player class.
 * 
 * The Player class represents a player in the game, including their name,
 * color, and score.
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <QObject>
#include <QColor>
#include <QString>

/**
 * @class Player
 * @brief Represents a player in the Dots and Boxes game.
 * 
 * This class stores player information including name, color, and score.
 * It also provides methods to update the score and reset player state.
 */
class Player : public QObject
{
    Q_OBJECT

public:
    explicit Player(const QString& name, const QColor& color, QObject* parent = nullptr);
    
    ~Player() override;

    QString getName() const { return m_name; }

    void setName(const QString& name) { m_name = name; }

    QColor getColor() const { return m_color; }

    void setColor(const QColor& color) { m_color = color; }

    int getScore() const { return m_score; }

    void addScore(int points) { m_score += points; }

    void resetScore() { m_score = 0; }

    int getPlayerId() const { return m_playerId; }
    void setPlayerId(int id) { m_playerId = id; }

signals:
    void scoreChanged(int newScore);

private:
    QString m_name;   ///< Player's name
    QColor m_color;   ///< Player's color
    int m_score;      ///< Player's score (number of boxes claimed)
    int m_playerId;
};

#endif // PLAYER_H