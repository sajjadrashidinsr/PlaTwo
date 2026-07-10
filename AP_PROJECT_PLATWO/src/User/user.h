#ifndef USER_H
#define USER_H

#include <QString>
#include <QVector>

struct GameRecord {

    QString opponentUsername;
    QString date;
    QString role; // "Host" or "Guest"
    QString winner;
    int finalScore;

};

class user
{

public:
    user();

    QString name;
    QString username;
    QString phone;
    QString email;
    QString passwordHash;


    int dotsAndBoxesScore = 0;
    int nineMensMorrisScore = 0;
    int fanoronaScore = 0;

    QVector<GameRecord> history;

};


#endif // USER_H
