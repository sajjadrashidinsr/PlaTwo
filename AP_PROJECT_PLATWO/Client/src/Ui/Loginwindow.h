#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <memory>

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

    user* getCurrentUser() const { return m_currentUser.get(); }

private slots:
    void onConnectedToServer();
    void onServerDisconnected();
    void onServerError(const QString& error);
    void onMainMenuClosed();
    void onLoginSuccessful(std::shared_ptr<user> loggedInUser);   // ← CHANGED

private:
    Ui::Loginwindow *ui;
    ClientManager* clientManager;
    loginPage* m_loginPage;
    signuppage* m_signupPage;
    forgotpasspage* m_forgotPage;

    std::unique_ptr<user> m_currentUser;
    MainMenuWindow* m_mainMenuWindow = nullptr;

    bool tryConnectToServer();
    void showLoginWindow();
};

#endif // LOGINWINDOW_H