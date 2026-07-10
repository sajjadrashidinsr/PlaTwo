#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include <QAction>
#include "storage_manager.h"
#include "auth_manager.h"
#include "user.h"

namespace Ui {
class loginPage;
}

class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(storage_manager* storage, QWidget *parent = nullptr);
    ~LoginPage();

private:
    Ui::loginPage *ui;
    storage_manager* storageManager;
    QAction *togglePasswordAction;
    bool passwordVisible = false;


signals:
    void createAccountClicked();

    void forgotPasswordClicked();

    void loginSuccessful(user* user);


private slots:
    void updateLoginButton();

    void on_btnLogin_clicked();
};

#endif // LOGINPAGE_H
