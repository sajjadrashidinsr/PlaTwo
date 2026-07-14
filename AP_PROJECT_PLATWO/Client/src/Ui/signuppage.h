#ifndef SIGNUPPAGE_H
#define SIGNUPPAGE_H

#include <QWidget>
#include <QAction>

// Forward declaration
class ClientManager;
class user;

namespace Ui {
class signuppage;
}

class signuppage : public QWidget {
    Q_OBJECT

public:
    explicit signuppage(ClientManager* client, QWidget *parent = nullptr);
    ~signuppage();

signals:
    void loginClicked();
    void signUpSuccessful();

private slots:
    void updateCreateButton();
    void onRegisterResponse(bool success, const QString& message);

private:
    Ui::signuppage *ui;
    ClientManager* clientManager;
    QAction *passwordAction;
    QAction *confirmpasswordAction;
    bool passwordVisible = false;
    bool confirmpasswordVisible = false;

private slots:
    void on_btnSubmitSignUp_clicked();
};

#endif // SIGNUPPAGE_H