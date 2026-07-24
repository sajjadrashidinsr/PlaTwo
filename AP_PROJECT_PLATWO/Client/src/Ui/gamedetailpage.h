#ifndef GAMEDETAILPAGE_H
#define GAMEDETAILPAGE_H

#include <QWidget>
#include <QPointer>
#include <memory>  // ← ADD THIS
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
    explicit GameDetailPage(user* currentUser, QPointer<ClientManager> client, QWidget *parent = nullptr);
    ~GameDetailPage();

    void setUser(user* u);
    void setGameType(GameType type);
    void loadData();
    void loadHistory();

signals:
    void backToMenu();
    void startNewGame(GameType gameType, bool isHost);

private slots:
    void onBackClicked();
    void onStartNewGameClicked();
    void onGetUserResponse(bool success, std::shared_ptr<user> userData, const QString& message);  // ← CHANGED

private:
    Ui::GameDetailPage *ui;
    user* currentUser;
    QPointer<ClientManager> clientManager;
    GameType currentGameType;
};

#endif // GAMEDETAILPAGE_H