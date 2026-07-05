#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include "loginpage.h"
#include "signuppage.h"
#include "forgotpasspage.h"
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui {
class Loginwindow;
}
QT_END_NAMESPACE

class Loginwindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit Loginwindow(QWidget *parent = nullptr);
    ~Loginwindow() override;

private:

    Ui::Loginwindow *ui;

    LoginPage *loginPage;

    signuppage *signupPage;

    forgotpasspage *forgotPage;
};
#endif // LOGINWINDOW_H
