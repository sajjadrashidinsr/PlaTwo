#include "forgotpasspage.h"
#include "ui_forgotpasspage.h"
#include <QAction>
#include <QIcon>
#include <QMessageBox>

forgotpasspage::forgotpasspage(storage_manager* storage,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::forgotpasspage),
    Storage(storage)
{
    ui->setupUi(this);

    ui->lineEdit_username->addAction(
        QIcon(":/icons/user.png"),
        QLineEdit::LeadingPosition);

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

    connect(ui->pushButton_newpass,
            &QPushButton::clicked,
            this,
            &forgotpasspage::on_btnChangePassword_clicked);

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
    bool ok = !ui->lineEdit_phone->text().trimmed().isEmpty() &&
              !ui->lineEdit_username->text().trimmed().isEmpty();

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

void forgotpasspage::on_btnChangePassword_clicked() {
    QString username = ui->lineEdit_username->text().trimmed();
    QString phone = ui->lineEdit_phone->text().trimmed();
    QString newPassword = ui->lineEdit_newpass->text();
    QString confirmnewpass = ui->lineEdit_confirm_newpass->text();

    if (username.isEmpty() || phone.isEmpty() || newPassword.isEmpty()) {
        QMessageBox::warning(this, "EROR", "pleae fill all blanks.");
        return;
    }

    user* user = Storage->getuser(username);
    if (!user) {
        QMessageBox::critical(this, "EROR", "username does not found.");
        return;
    }

    if (AuthManager::verifyPhoneForRecovery(user->phone, phone)) {
        if (newPassword.length() < 8) {
            QMessageBox::warning(this, "EROR", "passeord must be more than 8 characters.");
            delete user;
            return;
        }

        user->passwordHash = AuthManager::hashPassword(newPassword);

        if (Storage->updateuser(*user)) {
            QMessageBox::information(this, "successfull", "password changed.");

            ui->lineEdit_username->clear();
            ui->lineEdit_phone->clear();
            ui->lineEdit_newpass->clear();
            ui->lineEdit_confirm_newpass->clear();

            emit passwordChanged();
        } else {
            QMessageBox::critical(this, "EROR", "can not connect to the database.");
        }
    } else {
        QMessageBox::critical(this, "EROR", "phone number does not exist.");
    }

    delete user;
}

forgotpasspage::~forgotpasspage()
{
    delete ui;
}
