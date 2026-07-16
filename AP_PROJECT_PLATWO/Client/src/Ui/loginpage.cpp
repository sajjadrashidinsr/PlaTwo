#include "loginpage.h"
#include "ui_loginpage.h"
#include "client_manager.h"
#include "user.h"
#include <QAction>
#include <QIcon>
#include <QMessageBox>

loginPage::loginPage(ClientManager* client, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::loginPage)
    , clientManager(client) {

    ui->setupUi(this);

    ui->lineEdit_username->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);

    ui->lineEdit_username->setMaxLength(24);

    ui->lineEdit_username->setFocus();

    ui->lineEdit_password->addAction(QIcon(":/icons/lock.png"), QLineEdit::LeadingPosition);

    ui->lineEdit_password->setMaxLength(32);

    togglePasswordAction = ui->lineEdit_password->addAction(
        QIcon(":/icons/eye.png"), QLineEdit::TrailingPosition);

    connect(ui->lineEdit_username, &QLineEdit::textChanged,
            this, &loginPage::updateLoginButton);

    connect(ui->lineEdit_password, &QLineEdit::textChanged,
            this, &loginPage::updateLoginButton);

    updateLoginButton();

    connect(togglePasswordAction, &QAction::triggered, [this]() {
        passwordVisible = !passwordVisible;
        ui->lineEdit_password->setEchoMode(passwordVisible ? QLineEdit::Normal : QLineEdit::Password);
        togglePasswordAction->setIcon(QIcon(passwordVisible ? ":/icons/eye_off.png" : ":/icons/eye.png"));
    });

    connect(ui->pushButton_create_sign, &QPushButton::clicked, this, &loginPage::createAccountClicked);

    connect(ui->pushButton_forgot, &QPushButton::clicked, this, &loginPage::forgotPasswordClicked);

    connect(ui->pushButton_login, &QPushButton::clicked, this, &loginPage::on_btnLogin_clicked);

    connect(clientManager, &ClientManager::loginResponse,
            this, &loginPage::onLoginResponse);

}

void loginPage::updateLoginButton() {
    bool ok = !ui->lineEdit_username->text().trimmed().isEmpty() &&
              !ui->lineEdit_password->text().trimmed().isEmpty();
    ui->pushButton_login->setEnabled(ok);
}

void loginPage::on_btnLogin_clicked() {
    QString username = ui->lineEdit_username->text().trimmed();
    QString password = ui->lineEdit_password->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please enter username and password.");
        return;
    }

    if (!clientManager->isConnected()) {
        QMessageBox::warning(this, "Connection Error",
                             "Not connected to server. Please check the server is running.");
        return;
    }

    ui->pushButton_login->setEnabled(false);
    ui->pushButton_login->setText("Logging in...");

    clientManager->sendLogin(username, password);
}

void loginPage::onLoginResponse(bool success, user* userData, const QString& message) {
    ui->pushButton_login->setEnabled(true);
    ui->pushButton_login->setText("LOGIN");

    if (success && userData) {
        emit loginSuccessful(userData);
    } else {
        QMessageBox::critical(this, "Login Failed", message);
        if (userData) {
            delete userData;
        }
    }
}

loginPage::~loginPage() {
    delete ui;
}