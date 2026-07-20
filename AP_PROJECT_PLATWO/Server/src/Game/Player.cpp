#include "Player.h"

Player::Player(const QString& name, const QColor& color, QObject* parent)
    : QObject(parent)
    , m_name(name)
    , m_color(color)
    , m_score(0)
    , m_playerId(0)
{
}

Player::~Player()
{
}