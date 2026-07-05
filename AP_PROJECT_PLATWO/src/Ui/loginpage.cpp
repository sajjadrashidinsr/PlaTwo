#include "loginpage.h"
#include "ui_loginpage.h"
#include "forgotpasspage.h"
#include <QAction>
#include <QIcon>

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::loginPage)
{
    ui->setupUi(this);

    ui->lineEdit_username->addAction(
        QIcon(":/icons/user.png"),
        QLineEdit::LeadingPosition
        );

    ui->lineEdit_username->setMaxLength(24);

    ui->lineEdit_username->setFocus();

    ui->lineEdit_password->addAction(
        QIcon(":/icons/lock.png"),
        QLineEdit::LeadingPosition
        );

    ui->lineEdit_password->setMaxLength(32);

    togglePasswordAction =
        ui->lineEdit_password->addAction(
            QIcon(":/icons/eye.png"),
            QLineEdit::TrailingPosition
            );

    connect(ui->lineEdit_username,
            &QLineEdit::textChanged,
            this,
            &LoginPage::updateLoginButton);

    connect(ui->lineEdit_password,
            &QLineEdit::textChanged,
            this,
            &LoginPage::updateLoginButton);

    updateLoginButton();

    connect(togglePasswordAction,
            &QAction::triggered,
            this,
            [this]()
            {
                passwordVisible = !passwordVisible;

                if(passwordVisible)
                {
                    ui->lineEdit_password->setEchoMode(QLineEdit::Normal);
                    togglePasswordAction->setIcon(QIcon(":/icons/eye_off.png"));
                }
                else
                {
                    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
                    togglePasswordAction->setIcon(QIcon(":/icons/eye.png"));
                }
            });

    connect(ui->pushButton_create_sign,
            &QPushButton::clicked,
            this,
            &LoginPage::createAccountClicked);

    connect(ui->pushButton_forgot,
            &QPushButton::clicked,
            this,
            &LoginPage::forgotPasswordClicked);

}

void LoginPage::updateLoginButton(){
    bool ok =
            !ui->lineEdit_username->text().trimmed().isEmpty()&&
            !ui->lineEdit_password->text().trimmed().isEmpty()&&
            ui->lineEdit_password->text().length()>=8;



    ui->pushButton_login->setEnabled(ok);
}



LoginPage::~LoginPage()
{
    delete ui;
}
