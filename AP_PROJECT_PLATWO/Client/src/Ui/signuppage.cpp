#include "signuppage.h"
#include "ui_signuppage.h"
#include "client_manager.h"  // ✅ اضافه کردن include کامل
#include "user.h"            // ✅ اضافه کردن include کامل
#include <QAction>
#include <QIcon>
#include <QMessageBox>

signuppage::signuppage(ClientManager* client, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::signuppage)
    , clientManager(client) {

    ui->setupUi(this);

    ui->lineEdit_name->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);
    ui->lineEdit_username->addAction(QIcon(":/icons/user.png"), QLineEdit::LeadingPosition);
    ui->lineEdit_username->setMaxLength(24);
    ui->lineEdit_email->addAction(QIcon(":/icons/email.png"), QLineEdit::LeadingPosition);
    ui->lineEdit_password->addAction(QIcon(":/icons/lock.png"), QLineEdit::LeadingPosition);

    ui->lineEdit_confirmpassword->setEchoMode(QLineEdit::Password);
    ui->lineEdit_confirmpassword->addAction(QIcon(":/icons/lock.png"), QLineEdit::LeadingPosition);

    passwordAction = ui->lineEdit_password->addAction(QIcon(":/icons/eye.png"), QLineEdit::TrailingPosition);
    confirmpasswordAction = ui->lineEdit_confirmpassword->addAction(QIcon(":/icons/eye.png"), QLineEdit::TrailingPosition);

    connect(passwordAction, &QAction::triggered, [this]() {
        passwordVisible = !passwordVisible;
        ui->lineEdit_password->setEchoMode(passwordVisible ? QLineEdit::Normal : QLineEdit::Password);
        passwordAction->setIcon(QIcon(passwordVisible ? ":/icons/eye_off.png" : ":/icons/eye.png"));
    });

    connect(confirmpasswordAction, &QAction::triggered, [this]() {
        confirmpasswordVisible = !confirmpasswordVisible;
        ui->lineEdit_confirmpassword->setEchoMode(confirmpasswordVisible ? QLineEdit::Normal : QLineEdit::Password);
        confirmpasswordAction->setIcon(QIcon(confirmpasswordVisible ? ":/icons/eye_off.png" : ":/icons/eye.png"));
    });

    ui->pushButton_create_account->setEnabled(false);

    connect(ui->lineEdit_username, &QLineEdit::textChanged, this, &signuppage::updateCreateButton);
    connect(ui->lineEdit_email, &QLineEdit::textChanged, this, &signuppage::updateCreateButton);
    connect(ui->lineEdit_password, &QLineEdit::textChanged, this, &signuppage::updateCreateButton);
    connect(ui->lineEdit_confirmpassword, &QLineEdit::textChanged, this, &signuppage::updateCreateButton);
    updateCreateButton();

    connect(ui->pushButton_login_back, &QPushButton::clicked, this, &signuppage::loginClicked);
    connect(ui->pushButton_create_account, &QPushButton::clicked, this, &signuppage::on_btnSubmitSignUp_clicked);

    connect(clientManager, &ClientManager::registerResponse,
            this, &signuppage::onRegisterResponse);
}

void signuppage::updateCreateButton() {
    bool ok = !ui->lineEdit_username->text().trimmed().isEmpty() &&
              !ui->lineEdit_name->text().trimmed().isEmpty() &&
              !ui->lineEdit_phone->text().trimmed().isEmpty() &&
              !ui->lineEdit_email->text().trimmed().isEmpty() &&
              !ui->lineEdit_password->text().trimmed().isEmpty() &&
              !ui->lineEdit_confirmpassword->text().trimmed().isEmpty() &&
              ui->lineEdit_password->text() == ui->lineEdit_confirmpassword->text();

    ui->pushButton_create_account->setEnabled(ok);
}

void signuppage::on_btnSubmitSignUp_clicked() {
    user newUser;  // ✅ حالا type کامل است
    newUser.name = ui->lineEdit_name->text().trimmed();
    newUser.username = ui->lineEdit_username->text().trimmed();
    newUser.phone = ui->lineEdit_phone->text().trimmed();
    newUser.email = ui->lineEdit_email->text().trimmed();
    QString password = ui->lineEdit_password->text();
    QString confirmpass = ui->lineEdit_confirmpassword->text();

    if (newUser.name.isEmpty() || newUser.username.isEmpty() ||
        newUser.phone.isEmpty() || newUser.email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Fill all blanks.");
        return;
    }

    if (password != confirmpass) {
        QMessageBox::warning(this, "Error", "Passwords do not match!");
        return;
    }

    if (!clientManager->isConnected()) {
        QMessageBox::warning(this, "Connection Error",
                             "Not connected to server. Please check the server is running.");
        return;
    }

    // رمز را بدون هش به سرور می‌فرستیم، سرور هش می‌کند
    newUser.passwordHash = password;

    ui->pushButton_create_account->setEnabled(false);
    ui->pushButton_create_account->setText("Creating...");

    clientManager->sendRegister(newUser);
}

void signuppage::onRegisterResponse(bool success, const QString& message) {
    ui->pushButton_create_account->setEnabled(true);
    ui->pushButton_create_account->setText("Create Account");

    if (success) {
        QMessageBox::information(this, "Success", "Sign up successful. Log in to your account.");
        ui->lineEdit_name->clear();
        ui->lineEdit_username->clear();
        ui->lineEdit_phone->clear();
        ui->lineEdit_email->clear();
        ui->lineEdit_password->clear();
        ui->lineEdit_confirmpassword->clear();
        emit signUpSuccessful();
    } else {
        QMessageBox::critical(this, "Error", message);
    }
}

signuppage::~signuppage() {
    delete ui;
}