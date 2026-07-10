#include "loginpage.h"
#include "ui_loginpage.h"
#include "forgotpasspage.h"
#include <QAction>
#include <QIcon>
#include <QMessageBox>

LoginPage::LoginPage(storage_manager* storage,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::loginPage),
    storageManager(storage)
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

    connect(ui->pushButton_login,
            &QPushButton::clicked,
            this,
            &LoginPage::on_btnLogin_clicked);

}

void LoginPage::updateLoginButton(){
    bool ok =
        !ui->lineEdit_username->text().trimmed().isEmpty()&&
        !ui->lineEdit_password->text().trimmed().isEmpty()&&
        ui->lineEdit_password->text().length()>=8;



    ui->pushButton_login->setEnabled(ok);
}

void LoginPage::on_btnLogin_clicked() {
    QString username = ui->lineEdit_username->text().trimmed();
    QString password = ui->lineEdit_password->text();

    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "EROR", "Plese Enter username and password.");
        return;
    }

    user* user = storageManager->getuser(username);
    if (!user) {
        QMessageBox::critical(this, "EROR", "username not found!");
        return;
    }

    QString hashedInput = AuthManager::hashPassword(password);
    if (user->passwordHash == hashedInput) {
        emit loginSuccessful(user);
    } else {
        QMessageBox::critical(this, "خطا", "رمز عبور اشتباه است.");
        delete user;
    }
}

LoginPage::~LoginPage()
{
    delete ui;
}
