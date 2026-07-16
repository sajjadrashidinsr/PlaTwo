#include "editprofilepage.h"
#include "ui_editprofilepage.h"
#include "client_manager.h"
#include <QMessageBox>
#include <QRegularExpression>

EditProfilePage::EditProfilePage(user* currentUser, ClientManager* client, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::EditProfilePage)
    , currentUser(currentUser)
    , clientManager(client)
{
    ui->setupUi(this);

    ui->usernameEdit->setReadOnly(false);

    connect(ui->saveButton, &QPushButton::clicked, this, &EditProfilePage::onSaveClicked);

    connect(ui->cancelButton, &QPushButton::clicked, this, &EditProfilePage::onCancelClicked);

    connect(clientManager, &ClientManager::updateUserResponse,
            this, &EditProfilePage::onUpdateUserResponse);

}

EditProfilePage::~EditProfilePage()
{
    delete ui;
}

void EditProfilePage::setUser(user* u)
{
    currentUser = u;
    if (currentUser) {
        loadUserData();
    }
}

void EditProfilePage::loadUserData()
{
    if (!currentUser) return;

    ui->nameEdit->setText(currentUser->name);
    ui->usernameEdit->setText(currentUser->username);
    ui->phoneEdit->setText(currentUser->phone);
    ui->emailEdit->setText(currentUser->email);
    ui->passwordEdit->clear();
    ui->confirmPasswordEdit->clear();
}

bool EditProfilePage::validateInputs()
{

    if (ui->nameEdit->text().trimmed().isEmpty() || ui->usernameEdit->text().trimmed().isEmpty() ||
        ui->phoneEdit->text().trimmed().isEmpty() || ui->emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "All fields are required.");
        return false;
    }

    QString phone = ui->phoneEdit->text().trimmed();
    QRegularExpression phoneRegex("^09\\d{9}$");
    if (!phoneRegex.match(phone).hasMatch()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a valid phone number (e.g. 09xxxxxxxxx).");
        return false;
    }


    QString email = ui->emailEdit->text().trimmed();
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    if (!emailRegex.match(email).hasMatch()) {
        QMessageBox::warning(this, "Validation Error", "Please enter a valid email address.");
        return false;
    }

    QString password = ui->passwordEdit->text();
    QString confirm = ui->confirmPasswordEdit->text();

    if (!password.isEmpty() || !confirm.isEmpty()) {
        if (password.length() < 8) {
            QMessageBox::warning(this, "Validation Error", "Password must be at least 8 characters.");
            return false;
        }
        if (password != confirm) {
            QMessageBox::warning(this, "Validation Error", "Passwords do not match.");
            return false;
        }
    }

    return true;
}

void EditProfilePage::onSaveClicked()
{
    if (!validateInputs()) return;

    if (!clientManager->isConnected()) {
        QMessageBox::warning(this, "Connection Error", "Not connected to server.");
        return;
    }

    QString oldUsername = currentUser->username;

    user updatedUser = *currentUser;
    updatedUser.name = ui->nameEdit->text();
    updatedUser.username = ui->usernameEdit->text();
    updatedUser.phone = ui->phoneEdit->text();
    updatedUser.email = ui->emailEdit->text();

    QString password = ui->passwordEdit->text();
    if (password.isEmpty()) {

        updatedUser.passwordHash = "";
    } else {

        updatedUser.passwordHash = password;
    }

    ui->saveButton->setEnabled(false);
    ui->saveButton->setText("Saving...");

    clientManager->sendUpdateUser(oldUsername, updatedUser);

}

void EditProfilePage::onUpdateUserResponse(bool success, const QString& message) {
    ui->saveButton->setEnabled(true);
    ui->saveButton->setText("Save");

    if (success) {

        clientManager->sendGetUser(ui->usernameEdit->text());

        QMessageBox::information(this, "Success", "Profile updated successfully.");
        emit profileUpdated();
        emit backToMenu();
    } else {
        QMessageBox::critical(this, "Error", "Failed to update profile: " + message);
    }
}

void EditProfilePage::onCancelClicked()
{
    emit backToMenu();
}