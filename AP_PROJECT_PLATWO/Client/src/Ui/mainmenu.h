#ifndef MAINMENU_H
#define MAINMENU_H

#include <QWidget>
#include "user.h"

namespace Ui {
class MainMenu;
}

class MainMenu : public QWidget
{
    Q_OBJECT

public:
    explicit MainMenu(QWidget *parent = nullptr);
    ~MainMenu();

    void setUserInfo(user* loggedInUser);

signals:
    void gameSelected(int gameId);        // 0: Dots&Boxes, 1: NineMen'sMorris, 2: Fanorona
    void editProfileRequested();
    void exitRequested();

private slots:
    void onGameButtonClicked();
    void onEditProfileClicked();
    void onExitClicked();

private:
    Ui::MainMenu *ui;
    user* currentUser = nullptr;
};

#endif // MAINMENU_H