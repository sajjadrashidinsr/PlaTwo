#include "joingamedialog.h"
#include "ui_joingamedialog.h"
#include "client_manager.h"
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QIntValidator>

JoinGameDialog::JoinGameDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::JoinGameDialog)
{
    ui->setupUi(this);

    QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    ui->ipEdit->setValidator(new QRegularExpressionValidator(ipRegex, this));

    connect(ui->portSpin, &QSpinBox::editingFinished, this, &JoinGameDialog::onPortEditingFinished);
    connect(ui->connectButton, &QPushButton::clicked, this, &JoinGameDialog::onConnectClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    m_lastValidPort = static_cast<quint16>(ui->portSpin->value());
}

JoinGameDialog::~JoinGameDialog()
{
    delete ui;
}

void JoinGameDialog::onPortEditingFinished()
{
    int val = ui->portSpin->value();
    if (val >= 1024 && val <= 65535)
        m_lastValidPort = static_cast<quint16>(val);
    else
        ui->portSpin->setValue(m_lastValidPort);
}

bool validateJoinInputs(const Ui::JoinGameDialog *ui)
{
    QString ip = ui->ipEdit->text().trimmed();
    if (ip.isEmpty() || !ui->ipEdit->hasAcceptableInput()) {
        QMessageBox::warning(nullptr, "Error", "Please enter a valid IPv4 address.");
        return false;
    }

    if (ui->portSpin->value() < 1024 || ui->portSpin->value() > 65535) {
        QMessageBox::warning(nullptr, "Error", "Port must be between 1024 and 65535.");
        return false;
    }

    return true;
}

void JoinGameDialog::onConnectClicked()
{
    if (!validateJoinInputs(ui))
        return;

    if (!m_clientManager) {
        QMessageBox::critical(this, "Error", "Client manager not set.");
        return;
    }

    if (!m_clientManager->isConnected()) {
        QMessageBox::warning(this, "Connection Error",
                             "Not connected to server. Please check the server is running.");
        return;
    }

    QString ip = ui->ipEdit->text().trimmed();
    quint16 port = static_cast<quint16>(ui->portSpin->value());
    QString password = ui->passwordEdit->text();

    ui->connectButton->setEnabled(false);
    ui->connectButton->setText("Connecting...");

    connect(m_clientManager, &ClientManager::roomJoined,
            this, [this](bool success, const Room& room, const QString& message) {
                disconnect(m_clientManager, &ClientManager::roomJoined, this, nullptr);
                disconnect(m_clientManager, &ClientManager::roomError, this, nullptr);

                ui->connectButton->setEnabled(true);
                ui->connectButton->setText("Connect");

                if (success) {
                    emit joinSuccess(room);
                    accept();
                } else {
                    QMessageBox::critical(this, "Join Failed", message);
                    emit joinFailed(message);
                }
            });

    connect(m_clientManager, &ClientManager::roomError,
            this, [this](const QString& error) {
                disconnect(m_clientManager, &ClientManager::roomJoined, this, nullptr);
                disconnect(m_clientManager, &ClientManager::roomError, this, nullptr);

                ui->connectButton->setEnabled(true);
                ui->connectButton->setText("Connect");
                QMessageBox::critical(this, "Room Error", error);
                emit joinFailed(error);
            });

    m_clientManager->joinRoom(ip, port, password);
}