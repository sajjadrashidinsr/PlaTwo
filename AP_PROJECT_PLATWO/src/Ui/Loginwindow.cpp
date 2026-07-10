#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "loginpage.h"
#include "signuppage.h"
#include "forgotpasspage.h"
#include <QFile>
#include <QMessageBox>


Loginwindow::Loginwindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Loginwindow)
{
    ui->setupUi(this);

    storageManager = new storage_manager();

    ui->stackedWidget->setCurrentWidget(ui->pageLogin);

    loginPage = new LoginPage(storageManager,this);

    ui->verticalLayoutPageLogin->addWidget(loginPage);

    signupPage = new signuppage(storageManager,this);

    ui->verticalLayoutPagesignup->addWidget(signupPage);

    forgotPage = new forgotpasspage(storageManager,this);

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
    connect(signupPage,
            &signuppage::signUpSuccessful,
            this,
            [this]() {
                ui->stackedWidget->setCurrentWidget(ui->pageLogin);
            }
            );

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

    connect(forgotPage,
            &forgotpasspage::passwordChanged,
            this,
            [this]() {
                ui->stackedWidget->setCurrentWidget(ui->pageLogin);
            }
            );

    connect(loginPage,
            &LoginPage::loginSuccessful,
            this,
            [this](user* user) {
                QMessageBox::information(this, "login successfull ", "Welcome " + user->name + "!\n(اینجا منوی اصلی باز خواهد شد)");
            }
            );
}






Loginwindow::~Loginwindow()
{
    delete storageManager;
    delete ui;
}
