#ifndef MAINMENUWINDOW_H
#define MAINMENUWINDOW_H

#include <QWidget>
#include <QPointer>
#include <memory>
#include "user.h"
#include "mainmenu.h"
#include "gamedetailpage.h"
#include "editprofilepage.h"
#include "GameWidget/GameWidget.h"
#include "Dialogs/waitingroomdialog.h"
#include "../Models/room.h"

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

    void onGameStarted(const QJsonObject& data);
    void onGameStateReceived(const QJsonObject& state);
    void onGameOverReceived(const QJsonObject& data);
    void onGameAborted(const QString& message);
    void onLeaveGame();

private:
    void showGameWidget(int boardSize = 5);

    Ui::MainMenuWindow *ui;
    user* currentUser;  // ✅ نگهداری raw pointer (مدیریت توسط Loginwindow)
    QPointer<ClientManager> clientManager;

    MainMenu *mainMenuWidget;
    GameDetailPage *gameDetailPage;
    EditProfilePage *editProfilePage;

    QPointer<GameWidget> m_gameWidget;
    QPointer<WaitingRoomDialog> m_waitingDialog;
    Room m_currentRoom;
    bool m_isHost = false;
};

#endif // MAINMENUWINDOW_H