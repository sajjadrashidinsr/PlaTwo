#ifndef FORGOTPASSPAGE_H
#define FORGOTPASSPAGE_H

#include <QWidget>
#include <QAction>

class ClientManager;
class user;

namespace Ui {
class forgotpasspage;
}

class forgotpasspage : public QWidget {
    Q_OBJECT

public:
    explicit forgotpasspage(ClientManager* client, QWidget *parent = nullptr);
    ~forgotpasspage();

signals:
    void backToLoginClicked();
    void passwordChanged();

private slots:

    void toggleNewPassword();
    void toggleConfirmPassword();
    void checkVerifyButton();
    void checkChangeButton();
    void onForgotPasswordResponse(bool success, const QString& message);
    void onPasswordChangedResponse(bool success, const QString& message);

private:
    Ui::forgotpasspage *ui;
    ClientManager* clientManager;
    QAction* userAction;
    QAction *eyeNewPass;
    QAction *eyeConfirmPass;
    bool newPassVisible = false;
    bool confirmPassVisible = false;

private slots:
    void on_btnChangePassword_clicked();

};

#endif // FORGOTPASSPAGE_H