#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "loginpage.h"
#include "signuppage.h"
#include "forgotpasspage.h"
#include <QFile>


Loginwindow::Loginwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Loginwindow)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->pageLogin);

    loginPage = new LoginPage(this);

    ui->verticalLayoutPageLogin->addWidget(loginPage);

    signupPage = new signuppage(this);

    ui->verticalLayoutPagesignup->addWidget(signupPage);

    forgotPage = new forgotpasspage(this);

    ui->verticalLayoutPageforgotpass->addWidget(forgotPage);

    connect(loginPage,
            &LoginPage::createAccountClicked,
            this,
            [this]()
            {
                ui->stackedWidget->setCurrentWidget(ui->page_signup);
            });

    connect(signupPage,
            &signuppage::loginClicked,
            this,
            [this]()
            {
                ui->stackedWidget->setCurrentWidget(ui->pageLogin);
            });

    connect(loginPage,
            &LoginPage::forgotPasswordClicked,
            this,
            [this]()
            {
                ui->stackedWidget->setCurrentWidget(ui->pageforgotpass);
            });

    connect(forgotPage,
            &forgotpasspage::backToLoginClicked,
            this,
            [this]()
            {
                ui->stackedWidget->setCurrentWidget(ui->pageLogin);
            });

}

Loginwindow::~Loginwindow()
{
    delete ui;
}
