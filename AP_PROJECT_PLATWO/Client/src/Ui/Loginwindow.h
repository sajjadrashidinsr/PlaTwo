#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>

// Forward declarations - به جای include کامل
class ClientManager;
class loginPage;
class signuppage;
class forgotpasspage;

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

private:
    Ui::Loginwindow *ui;
    ClientManager* clientManager;
    loginPage* m_loginPage;
    signuppage* m_signupPage;
    forgotpasspage* m_forgotPage;

    bool tryConnectToServer();
};

#endif // LOGINWINDOW_H