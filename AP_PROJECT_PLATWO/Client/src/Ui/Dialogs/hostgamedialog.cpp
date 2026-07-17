#include "hostgamedialog.h"
#include "ui_hostgamedialog.h"
#include "client_manager.h"
#include <QMessageBox>
#include <QIntValidator>

HostGameDialog::HostGameDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HostGameDialog)
{
    ui->setupUi(this);

    connect(ui->limitedRadio, &QRadioButton::toggled, this, &HostGameDialog::onTimeControlToggled);
    connect(ui->portSpin, &QSpinBox::editingFinished, this, &HostGameDialog::onPortEditingFinished);
    connect(ui->createButton, &QPushButton::clicked, this, &HostGameDialog::onCreateClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    m_lastValidPort = static_cast<quint16>(ui->portSpin->value());
}

HostGameDialog::~HostGameDialog()
{
    delete ui;
}

void HostGameDialog::onTimeControlToggled(bool checked)
{
    ui->minutesSpin->setEnabled(checked);
    ui->secondsSpin->setEnabled(checked);
}

void HostGameDialog::onPortEditingFinished()
{
    int val = ui->portSpin->value();
    if (val >= 1024 && val <= 65535)
        m_lastValidPort = static_cast<quint16>(val);
    else
        ui->portSpin->setValue(m_lastValidPort);
}

bool validateInputs(const Ui::HostGameDialog *ui)
{
    if (ui->roomNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "Error", "Room name is required.");
        return false;
    }

    if (ui->portSpin->value() < 1024 || ui->portSpin->value() > 65535) {
        QMessageBox::warning(nullptr, "Error", "Port must be between 1024 and 65535.");
        return false;
    }

    int boardSize = ui->boardSizeCombo->currentIndex() + 3;
    if (boardSize < 3 || boardSize > 10) {
        QMessageBox::warning(nullptr, "Error", "Invalid board size.");
        return false;
    }

    return true;
}

void HostGameDialog::onCreateClicked()
{
    if (!validateInputs(ui))
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

    // Build settings
    GameSettings settings;
    settings.boardSize = ui->boardSizeCombo->currentIndex() + 3;
    settings.timed = ui->limitedRadio->isChecked();
    settings.minutes = ui->minutesSpin->value();
    settings.seconds = ui->secondsSpin->value();

    QString roomName = ui->roomNameEdit->text().trimmed();
    quint16 port = static_cast<quint16>(ui->portSpin->value());
    QString password = ui->passwordEdit->text();

    // Disable UI
    ui->createButton->setEnabled(false);
    ui->createButton->setText("Creating...");

    // Connect to client manager signals for this operation
    connect(m_clientManager, &ClientManager::roomCreated,
            this, [this](bool success, const Room& room, const QString& message) {
                // Disconnect to avoid multiple triggers
                disconnect(m_clientManager, &ClientManager::roomCreated, this, nullptr);
                disconnect(m_clientManager, &ClientManager::roomError, this, nullptr);

                ui->createButton->setEnabled(true);
                ui->createButton->setText("Create Room");

                if (success) {
                    emit roomCreationSuccess(room);
                    accept();
                } else {
                    QMessageBox::critical(this, "Room Creation Failed", message);
                    emit roomCreationFailed(message);
                }
            });

    connect(m_clientManager, &ClientManager::roomError,
            this, [this](const QString& error) {
                disconnect(m_clientManager, &ClientManager::roomCreated, this, nullptr);
                disconnect(m_clientManager, &ClientManager::roomError, this, nullptr);

                ui->createButton->setEnabled(true);
                ui->createButton->setText("Create Room");
                QMessageBox::critical(this, "Room Error", error);
                emit roomCreationFailed(error);
            });

    // Send create room request
    m_clientManager->createRoom(roomName, port, settings, password);
}