/**
 * @file Player.cpp
 * @brief Implementation of the Player class.
 */

#include "Player.h"

/**
 * @brief Constructor for Player.
 * @param name The player's name.
 * @param color The player's color.
 * @param parent Parent object.
 */
Player::Player(const QString& name, const QColor& color, QObject* parent)
    : QObject(parent)
    , m_name(name)
    , m_color(color)
    , m_score(0)
{
}

/**
 * @brief Destructor for Player.
 */
Player::~Player()
{
    // Qt handles deletion
}