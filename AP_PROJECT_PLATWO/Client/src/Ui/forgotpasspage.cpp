#include "forgotpasspage.h"
#include "ui_forgotpasspage.h"
#include "client_manager.h"
// #include "user.h"  // ❌ این خط را حذف کنید - چون مستقیماً استفاده نمی‌شود
#include <QAction>
#include <QIcon>
#include <QMessageBox>

forgotpasspage::forgotpasspage(ClientManager* client, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::forgotpasspage)
    , clientManager(client) {

    ui->setupUi(this);

    ui->lineEdit_username->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);
    userAction = ui->lineEdit_phone->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);
    ui->lineEdit_newpass->addAction(QIcon(":/icons/lock.png"), QLineEdit::LeadingPosition);
    ui->lineEdit_confirm_newpass->addAction(QIcon(":/icons/lock.png"), QLineEdit::LeadingPosition);

    ui->lineEdit_newpass->setEchoMode(QLineEdit::Password);
    ui->lineEdit_confirm_newpass->setEchoMode(QLineEdit::Password);

    eyeNewPass = ui->lineEdit_newpass->addAction(QIcon(":/icons/eye.png"), QLineEdit::TrailingPosition);
    eyeConfirmPass = ui->lineEdit_confirm_newpass->addAction(QIcon(":/icons/eye.png"), QLineEdit::TrailingPosition);

    connect(eyeNewPass, &QAction::triggered, this, &forgotpasspage::toggleNewPassword);
    connect(eyeConfirmPass, &QAction::triggered, this, &forgotpasspage::toggleConfirmPassword);
    connect(ui->pushButton_backlogin, &QPushButton::clicked, this, &forgotpasspage::backToLoginClicked);

    ui->pushButton_verify->setEnabled(false);
    connect(ui->lineEdit_phone, &QLineEdit::textChanged, this, &forgotpasspage::checkVerifyButton);

    ui->pushButton_newpass->setEnabled(false);
    connect(ui->lineEdit_newpass, &QLineEdit::textChanged, this, &forgotpasspage::checkChangeButton);
    connect(ui->lineEdit_confirm_newpass, &QLineEdit::textChanged, this, &forgotpasspage::checkChangeButton);
    connect(ui->pushButton_newpass, &QPushButton::clicked, this, &forgotpasspage::on_btnChangePassword_clicked);

    connect(clientManager, &ClientManager::forgotPasswordResponse,
            this, &forgotpasspage::onForgotPasswordResponse);
    connect(clientManager, &ClientManager::passwordChangedResponse,
            this, &forgotpasspage::onPasswordChangedResponse);
}

void forgotpasspage::toggleNewPassword() {
    newPassVisible = !newPassVisible;
    ui->lineEdit_newpass->setEchoMode(newPassVisible ? QLineEdit::Normal : QLineEdit::Password);
    eyeNewPass->setIcon(QIcon(newPassVisible ? ":/icons/eye_off.png" : ":/icons/eye.png"));
}

void forgotpasspage::toggleConfirmPassword() {
    confirmPassVisible = !confirmPassVisible;
    ui->lineEdit_confirm_newpass->setEchoMode(confirmPassVisible ? QLineEdit::Normal : QLineEdit::Password);
    eyeConfirmPass->setIcon(QIcon(confirmPassVisible ? ":/icons/eye_off.png" : ":/icons/eye.png"));
}

void forgotpasspage::checkVerifyButton() {
    bool ok = !ui->lineEdit_phone->text().trimmed().isEmpty() &&
              !ui->lineEdit_username->text().trimmed().isEmpty();
    ui->pushButton_verify->setEnabled(ok);
}

void forgotpasspage::checkChangeButton() {
    QString pass = ui->lineEdit_newpass->text();
    QString confirm = ui->lineEdit_confirm_newpass->text();
    bool ok = !pass.isEmpty() && !confirm.isEmpty() && pass == confirm;
    ui->pushButton_newpass->setEnabled(ok);
}

void forgotpasspage::on_btnChangePassword_clicked() {
    QString username = ui->lineEdit_username->text().trimmed();
    QString phone = ui->lineEdit_phone->text().trimmed();
    QString newPassword = ui->lineEdit_newpass->text();

    if (username.isEmpty() || phone.isEmpty() || newPassword.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please fill all blanks.");
        return;
    }

    if (!clientManager->isConnected()) {
        QMessageBox::warning(this, "Connection Error",
                             "Not connected to server. Please check the server is running.");
        return;
    }

    ui->pushButton_newpass->setEnabled(false);
    ui->pushButton_newpass->setText("Verifying...");

    clientManager->sendForgotPassword(username, phone);
}

void forgotpasspage::onForgotPasswordResponse(bool success, const QString& message) {
    if (success) {
        QString username = ui->lineEdit_username->text().trimmed();
        QString phone = ui->lineEdit_phone->text().trimmed();
        QString newPassword = ui->lineEdit_newpass->text();

        ui->pushButton_newpass->setText("Changing...");
        clientManager->sendChangePassword(username, phone, newPassword);
    } else {
        ui->pushButton_newpass->setEnabled(true);
        ui->pushButton_newpass->setText("Change Password");
        QMessageBox::critical(this, "Verification Failed", message);
    }
}

void forgotpasspage::onPasswordChangedResponse(bool success, const QString& message) {
    ui->pushButton_newpass->setEnabled(true);
    ui->pushButton_newpass->setText("Change Password");

    if (success) {
        QMessageBox::information(this, "Success", "Password changed successfully.");
        ui->lineEdit_username->clear();
        ui->lineEdit_phone->clear();
        ui->lineEdit_newpass->clear();
        ui->lineEdit_confirm_newpass->clear();
        emit passwordChanged();
    } else {
        QMessageBox::critical(this, "Error", message);
    }
}

forgotpasspage::~forgotpasspage() {
    delete ui;
}