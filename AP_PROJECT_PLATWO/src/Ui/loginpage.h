#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include <QAction>

namespace Ui {
class loginPage;
}

class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage();

private:
    Ui::loginPage *ui;

    QAction *togglePasswordAction;
    bool passwordVisible = false;


signals:
    void createAccountClicked();

    void forgotPasswordClicked();

private slots:
    void updateLoginButton();
};

#endif // LOGINPAGE_H
