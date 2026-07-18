#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include <QMetaType>

struct GameSettings
{
    int boardSize = 3;
    bool timed = false;
    int minutes = 0;
    int seconds = 0;

    int totalSeconds() const { return minutes * 60 + seconds; }
    bool isValid() const { return boardSize >= 3 && boardSize <= 10; }
};

Q_DECLARE_METATYPE(GameSettings)

#endif // GAMESETTINGS_H