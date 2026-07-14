#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include <QAction>

// Forward declaration
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
    void loginSuccessful(user* user);

private slots:
    void updateLoginButton();
    void onLoginResponse(bool success, user* userData, const QString& message);

private:
    Ui::loginPage *ui;
    ClientManager* clientManager;
    QAction *togglePasswordAction;
    bool passwordVisible = false;

private slots:
    void on_btnLogin_clicked();
};

#endif // LOGINPAGE_H