#ifndef GAMEDETAILPAGE_H
#define GAMEDETAILPAGE_H

#include <QWidget>
#include "user.h"

class ClientManager;

namespace Ui {
class GameDetailPage;
}

enum class GameType { DotsAndBoxes, NineMensMorris, Fanorona };

class GameDetailPage : public QWidget
{
    Q_OBJECT

public:
    explicit GameDetailPage(user* currentUser, ClientManager* client, QWidget *parent = nullptr);
    ~GameDetailPage();

    void setUser(user* u);
    void setGameType(GameType type);
    void loadData();

signals:
    void backToMenu();
    void startNewGame(GameType gameType, bool isHost);

private slots:
    void onBackClicked();
    void onStartNewGameClicked();
    void onGetUserResponse(bool success, user* userData, const QString& message);

private:
    Ui::GameDetailPage *ui;
    user* currentUser;
    ClientManager* clientManager;
    GameType currentGameType;
};

#endif // GAMEDETAILPAGE_H