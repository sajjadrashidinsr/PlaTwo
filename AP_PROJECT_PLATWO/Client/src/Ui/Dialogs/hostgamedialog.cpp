#include "hostgamedialog.h"
#include "ui_hostgamedialog.h"
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

    // Set initial port validity
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

    GameSettings settings;
    settings.boardSize = ui->boardSizeCombo->currentIndex() + 3;
    settings.timed = ui->limitedRadio->isChecked();
    settings.minutes = ui->minutesSpin->value();
    settings.seconds = ui->secondsSpin->value();

    emit createRoom(ui->roomNameEdit->text().trimmed(),
                    static_cast<quint16>(ui->portSpin->value()),
                    settings,
                    ui->passwordEdit->text());
    accept();
}