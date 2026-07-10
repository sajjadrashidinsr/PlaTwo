#include "signuppage.h"
#include "ui_signuppage.h"
#include <QAction>
#include <QIcon>
#include <QMessageBox>

signuppage::signuppage(storage_manager* storage,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::signuppage),
    storageManager(storage)
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

    connect(ui->pushButton_create_account,
            &QPushButton::clicked,
            this,
            &signuppage::on_btnSubmitSignUp_clicked);

}

void signuppage::updateCreateButton(){
    bool ok =
        !ui->lineEdit_username->text().trimmed().isEmpty() &&
        !ui->lineEdit_name->text().trimmed().isEmpty()&&
        !ui->lineEdit_phone->text().trimmed().isEmpty() &&
        !ui->lineEdit_email->text().trimmed().isEmpty() &&
        !ui->lineEdit_password->text().trimmed().isEmpty() &&
        !ui->lineEdit_confirmpassword->text().trimmed().isEmpty() &&
        ui->lineEdit_password->text() ==
            ui->lineEdit_confirmpassword->text() &&
        ui->lineEdit_password->text().length()>=8;


    ui->pushButton_create_account->setEnabled(ok);
}

void signuppage::on_btnSubmitSignUp_clicked() {
    user newUser;
    newUser.name = ui->lineEdit_name->text().trimmed();
    newUser.username = ui->lineEdit_username->text().trimmed();
    newUser.phone = ui->lineEdit_phone->text().trimmed();
    newUser.email = ui->lineEdit_email->text().trimmed();
    QString password = ui->lineEdit_password->text();
    QString confirmpass = ui->lineEdit_confirmpassword->text();

    if (newUser.name.isEmpty() || newUser.username.isEmpty() ||
        newUser.phone.isEmpty() || newUser.email.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "EROR", "Fill all blanks.");
        return;
    }

    if (!AuthManager::validateEmail(newUser.email)) {
        QMessageBox::warning(this, "EROR", "invalid email.");
        return;
    }

    if (!AuthManager::validatePhone(newUser.phone)) {
        QMessageBox::warning(this, "EROR", "invalid phone.");
        return;
    }

    if (password.length() < 8) {
        QMessageBox::warning(this, "EROR", "password must be more than 8 character.");
        return;
    }

    if (password!=confirmpass){
        QMessageBox::warning(this, "EROR", "password does not match!");
        return;
    }

    newUser.passwordHash = AuthManager::hashPassword(password);

    if (storageManager->registeruser(newUser)) {
        QMessageBox::information(this, "successfull", "sign up successful. Log in to your account..");


        ui->lineEdit_name->clear();
        ui->lineEdit_username->clear();
        ui->lineEdit_phone->clear();
        ui->lineEdit_email->clear();
        ui->lineEdit_password->clear();
        ui->lineEdit_confirmpassword->clear();


        emit signUpSuccessful();
    } else {
        QMessageBox::critical(this, "EROR", "this username already exists.");
    }
}

signuppage::~signuppage()
{
    delete ui;
}
