#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include <QAction>
#include <memory>   // ADD THIS

class ClientManager;
class user;

namespace Ui {
class loginPage;
}

class loginPage : public QWidget {
    Q_OBJECT

public:
    explicit loginPage(ClientManager* client, QWidget *parent = nullptr);
    ~loginPage();

signals:
    void createAccountClicked();
    void forgotPasswordClicked();
    void loginSuccessful(std::shared_ptr<user> userData);   // ← CHANGED TO shared_ptr

private slots:
    void updateLoginButton();
    void onLoginResponse(bool success, std::shared_ptr<user> userData, const QString& message);   // ← CHANGED

private:
    Ui::loginPage *ui;
    ClientManager* clientManager;
    QAction *togglePasswordAction;
    bool passwordVisible = false;

private slots:
    void on_btnLogin_clicked();
};

#endif // LOGINPAGE_H