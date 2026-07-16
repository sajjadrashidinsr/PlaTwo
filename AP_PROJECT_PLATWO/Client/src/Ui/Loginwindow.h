#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>

class ClientManager;
class loginPage;
class signuppage;
class forgotpasspage;
class user;
class MainMenuWindow;

namespace Ui {
class Loginwindow;
}

class Loginwindow : public QMainWindow {
    Q_OBJECT

public:
    explicit Loginwindow(QWidget *parent = nullptr);
    ~Loginwindow() override;

private slots:
    void onConnectedToServer();
    void onServerDisconnected();
    void onServerError(const QString& error);
    void onMainMenuClosed();

private:
    Ui::Loginwindow *ui;
    ClientManager* clientManager;
    loginPage* m_loginPage;
    signuppage* m_signupPage;
    forgotpasspage* m_forgotPage;


    user* m_currentUser = nullptr;
    MainMenuWindow* m_mainMenuWindow = nullptr;

    bool tryConnectToServer();
    void showLoginWindow();
};

#endif // LOGINWINDOW_H