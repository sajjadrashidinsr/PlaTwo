#include "signuppage.h"
#include "ui_signuppage.h"
#include <QAction>
#include <QIcon>

signuppage::signuppage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::signuppage)
{
    ui->setupUi(this);

    ui->lineEdit_name->addAction(
        QIcon(":/icons/user.png"),
        QLineEdit::LeadingPosition);

    ui->lineEdit_username->addAction(
        QIcon(":/icons/user.png"),
        QLineEdit::LeadingPosition);

    ui->lineEdit_username->setMaxLength(24);

    ui->lineEdit_email->addAction(
        QIcon(":/icons/email.png"),
        QLineEdit::LeadingPosition);

    ui->lineEdit_password->addAction(
        QIcon(":/icons/lock.png"),
        QLineEdit::LeadingPosition);

    ui->lineEdit_confirmpassword->setEchoMode(QLineEdit::Password);

    ui->lineEdit_confirmpassword->addAction(
        QIcon(":/icons/lock.png"),
        QLineEdit::LeadingPosition);

    passwordAction =
        ui->lineEdit_password->addAction(
        QIcon(":/icons/eye.png"),
        QLineEdit::TrailingPosition);

    confirmpasswordAction =
        ui->lineEdit_confirmpassword->addAction(
        QIcon(":/icons/eye.png"),
        QLineEdit::TrailingPosition);

    connect(passwordAction,
            &QAction::triggered,
            this,
            [this](){

                passwordVisible = !passwordVisible;

                ui->lineEdit_password->setEchoMode(
                    passwordVisible ?
                        QLineEdit::Normal :
                        QLineEdit::Password);

                passwordAction->setIcon(
                    QIcon(passwordVisible ?
                        ":/icons/eye_off.png" :
                        ":/icons/eye.png"));

            });

    connect(confirmpasswordAction,
            &QAction::triggered,
            this,
            [this](){

                confirmpasswordVisible = !confirmpasswordVisible;

                ui->lineEdit_confirmpassword->setEchoMode(
                    confirmpasswordVisible ?
                        QLineEdit::Normal :
                        QLineEdit::Password);

                confirmpasswordAction->setIcon(
                    QIcon(confirmpasswordVisible ?
                        ":/icons/eye_off.png" :
                        ":/icons/eye.png"));

            });

    ui->pushButton_create_account->setEnabled(false);

    connect(ui->lineEdit_username,
            &QLineEdit::textChanged,
            this,
            &signuppage::updateCreateButton);

    connect(ui->lineEdit_email,
            &QLineEdit::textChanged,
            this,
            &signuppage::updateCreateButton);

    connect(ui->lineEdit_password,
            &QLineEdit::textChanged,
            this,
            &signuppage::updateCreateButton);

    connect(ui->lineEdit_confirmpassword,
            &QLineEdit::textChanged,
            this,
            &signuppage::updateCreateButton);

    updateCreateButton();

    connect(ui->pushButton_login_back,
            &QPushButton::clicked,
            this,
            &signuppage::loginClicked);

}

void signuppage::updateCreateButton(){
    bool ok =
            !ui->lineEdit_username->text().trimmed().isEmpty() &&
            !ui->lineEdit_name->text().trimmed().isEmpty()&&
            !ui->lineEdit_email->text().trimmed().isEmpty() &&
            !ui->lineEdit_password->text().trimmed().isEmpty() &&
            !ui->lineEdit_confirmpassword->text().trimmed().isEmpty() &&
            ui->lineEdit_password->text() ==
            ui->lineEdit_confirmpassword->text() &&
            ui->lineEdit_password->text().length()>=8;


    ui->pushButton_create_account->setEnabled(ok);
}

signuppage::~signuppage()
{
    delete ui;
}
