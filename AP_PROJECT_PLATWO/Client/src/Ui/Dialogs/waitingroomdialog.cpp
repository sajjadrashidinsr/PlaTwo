#include "waitingroomdialog.h"
#include "ui_waitingroomdialog.h"
#include "client_manager.h"

#include <QClipboard>
#include <QApplication>
#include <QNetworkInterface>
#include <QMessageBox>
#include <QDebug>

WaitingRoomDialog::WaitingRoomDialog(bool isHost, const Room& room, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::WaitingRoomDialog)
    , m_isHost(isHost)
    , m_room(room)
{
    ui->setupUi(this);
    setWindowTitle(isHost ? "Waiting Room - Host" : "Waiting Room - Guest");
    updateInfo();

    connect(ui->copyButton, &QPushButton::clicked, this, &WaitingRoomDialog::onCopyAddressClicked);
    connect(ui->leaveButton, &QPushButton::clicked, this, &WaitingRoomDialog::onLeaveClicked);
}

WaitingRoomDialog::~WaitingRoomDialog()
{
    delete ui;
}

void WaitingRoomDialog::setClientManager(ClientManager* client)
{
    m_clientManager = client;
    if (m_clientManager) {
        // Connect to dynamic updates
        connect(m_clientManager, &ClientManager::playerJoined,
                this, &WaitingRoomDialog::onPlayerJoined);
        connect(m_clientManager, &ClientManager::playerLeft,
                this, &WaitingRoomDialog::onPlayerLeft);
        connect(m_clientManager, &ClientManager::roomError,
                this, &WaitingRoomDialog::onRoomError);
    }
}

QString getLocalIPv4()
{
    const QList<QNetworkInterface>& interfaces = QNetworkInterface::allInterfaces();
    for (const QNetworkInterface& iface : interfaces) {
        if (!(iface.flags() & QNetworkInterface::IsRunning) || (iface.flags() & QNetworkInterface::IsLoopBack))
            continue;
        QString name = iface.name().toLower();
        if (name.contains("vmware") || name.contains("virtual") || name.contains("docker") ||
            name.contains("vpn") || name.contains("tun") || name.contains("tap") ||
            name.contains("bluetooth") || name.contains("bridge"))
            continue;

        const QList<QNetworkAddressEntry>& entries = iface.addressEntries();
        for (const QNetworkAddressEntry& entry : entries) {
            if (entry.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                return entry.ip().toString();
            }
        }
    }
    return QString();
}

void WaitingRoomDialog::updateInfo()
{
    ui->statusLabel->setText(m_isHost ? "Waiting for a guest to join..." : "Connected to host. Waiting for game to start...");
    ui->hostLabel->setText(m_room.hostUsername.isEmpty() ? "(unknown)" : m_room.hostUsername);
    ui->guestLabel->setText(m_room.guestUsername.isEmpty() ? "(not connected)" : m_room.guestUsername);
    ui->boardLabel->setText(QString::number(m_room.gameSettings.boardSize) + "x" +
                            QString::number(m_room.gameSettings.boardSize));
    QString timeStr = m_room.gameSettings.timed ?
                          QString::number(m_room.gameSettings.minutes) + ":" +
                              QString("%1").arg(m_room.gameSettings.seconds, 2, 10, QChar('0')) :
                          "Unlimited";
    ui->timeLabel->setText(timeStr);

    QString ip = getLocalIPv4();
    if (ip.isEmpty())
        ip = "127.0.0.1";
    ui->ipLabel->setText(ip + ":" + QString::number(m_room.port));
}

void WaitingRoomDialog::onCopyAddressClicked()
{
    QString address = ui->ipLabel->text();
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(address);
    QMessageBox::information(this, "Copied", "Server address copied to clipboard:\n" + address);
}

void WaitingRoomDialog::onLeaveClicked()
{
    if (m_clientManager) {
        m_clientManager->leaveRoom();
    }
    emit leaveRoom();
    accept();
}

// ---------- Phase 2: Dynamic Updates ----------
void WaitingRoomDialog::onPlayerJoined(const QString& playerName)
{
    qDebug() << "[WaitingRoom] Player joined:" << playerName;
    m_room.guestUsername = playerName;
    updateInfo();
    // Optional: show a notification
    if (m_isHost) {
        ui->statusLabel->setText("Guest " + playerName + " has joined! Ready to start game.");
    }
}

void WaitingRoomDialog::onPlayerLeft(const QString& playerName)
{
    qDebug() << "[WaitingRoom] Player left:" << playerName;
    if (m_room.guestUsername == playerName) {
        m_room.guestUsername.clear();
        updateInfo();
        if (m_isHost) {
            ui->statusLabel->setText("Guest " + playerName + " left. Waiting for a new guest...");
        }
    }
}

void WaitingRoomDialog::onRoomError(const QString& error)
{
    QMessageBox::critical(this, "Room Error", error);
    // Optionally close the dialog
    reject();
}