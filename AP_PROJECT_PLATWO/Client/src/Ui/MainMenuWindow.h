#ifndef MAINMENUWINDOW_H
#define MAINMENUWINDOW_H

#include <QWidget>
#include "user.h"
#include "mainmenu.h"
#include "gamedetailpage.h"
#include "editprofilepage.h"

class ClientManager;

namespace Ui {
class MainMenuWindow;
}

class MainMenuWindow : public QWidget {
    Q_OBJECT

public:
    explicit MainMenuWindow(user* loggedInUser, ClientManager* client, QWidget *parent = nullptr);
    ~MainMenuWindow();

    void setUserInfo(user* loggedInUser);

protected:
    void showEvent(QShowEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

signals:
    void exitRequested();

private slots:
    void onGameSelected(int gameId);
    void onBackToMenu();
    void onStartNewGame(GameType gameType, bool isHost);
    void onEditProfileRequested();

private:
    Ui::MainMenuWindow *ui;
    user* currentUser;
    ClientManager* clientManager;

    MainMenu *mainMenuWidget;
    GameDetailPage *gameDetailPage;
    EditProfilePage *editProfilePage;
};

#endif // MAINMENUWINDOW_H