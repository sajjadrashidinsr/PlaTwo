#include "forgotpasspage.h"
#include "ui_forgotpasspage.h"
#include <QAction>
#include <QIcon>

forgotpasspage::forgotpasspage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::forgotpasspage)
{
    ui->setupUi(this);

    userAction =
        ui->lineEdit_phone->addAction(
        QIcon(":/icons/user.png"),
        QLineEdit::LeadingPosition);

    ui->lineEdit_newpass->addAction(
        QIcon(":/icons/lock.png"),
        QLineEdit::LeadingPosition);

    ui->lineEdit_confirm_newpass->addAction(
        QIcon(":/icons/lock.png"),
        QLineEdit::LeadingPosition);

    ui->lineEdit_newpass->setEchoMode(QLineEdit::Password);
    ui->lineEdit_confirm_newpass->setEchoMode(QLineEdit::Password);

    eyeNewPass =
        ui->lineEdit_newpass->addAction(
            QIcon(":/icons/eye.png"),
            QLineEdit::TrailingPosition);

    eyeConfirmPass =
        ui->lineEdit_confirm_newpass->addAction(
            QIcon(":/icons/eye.png"),
            QLineEdit::TrailingPosition);

    connect(eyeNewPass,
            &QAction::triggered,
            this,
            &forgotpasspage::toggleNewPassword);

    connect(eyeConfirmPass,
            &QAction::triggered,
            this,
            &forgotpasspage::toggleConfirmPassword);


    connect(ui->pushButton_backlogin,
            &QPushButton::clicked,
            this,
            &forgotpasspage::backToLoginClicked);

    ui->pushButton_verify->setEnabled(false);

    connect(ui->lineEdit_phone,
            &QLineEdit::textChanged,
            this,
            &forgotpasspage::checkVerifyButton);

    ui->pushButton_newpass->setEnabled(false);

    connect(ui->lineEdit_newpass,
            &QLineEdit::textChanged,
            this,
            &forgotpasspage::checkChangeButton);

    connect(ui->lineEdit_confirm_newpass,
            &QLineEdit::textChanged,
            this,
            &forgotpasspage::checkChangeButton);
}



void forgotpasspage::toggleNewPassword()
{
    newPassVisible = !newPassVisible;

    ui->lineEdit_newpass->setEchoMode(
        newPassVisible ?
            QLineEdit::Normal :
            QLineEdit::Password);

    eyeNewPass->setIcon(
        QIcon(newPassVisible ?
                  ":/icons/eye_off.png"
                             :
                  ":/icons/eye.png"));
}

void forgotpasspage::toggleConfirmPassword()
{
    confirmPassVisible = !confirmPassVisible;

    ui->lineEdit_confirm_newpass->setEchoMode(
        confirmPassVisible ?
            QLineEdit::Normal :
            QLineEdit::Password);

    eyeConfirmPass->setIcon(
        QIcon(confirmPassVisible ?
                  ":/icons/eye_off.png"
                                 :
                  ":/icons/eye.png"));
}

void forgotpasspage::checkVerifyButton()
{
    bool ok = !ui->lineEdit_phone->text().trimmed().isEmpty();

    ui->pushButton_verify->setEnabled(ok);
}

void forgotpasspage::checkChangeButton()
{
    QString pass = ui->lineEdit_newpass->text();
    QString confirm = ui->lineEdit_confirm_newpass->text();

    bool ok =
        !pass.isEmpty() &&
        !confirm.isEmpty() &&
        pass == confirm &&
        pass.length() >= 8;

    ui->pushButton_newpass->setEnabled(ok);
}

forgotpasspage::~forgotpasspage()
{
    delete ui;
}
