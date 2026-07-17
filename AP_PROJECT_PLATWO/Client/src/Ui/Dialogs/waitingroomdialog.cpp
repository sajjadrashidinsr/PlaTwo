#include "waitingroomdialog.h"
#include "ui_waitingroomdialog.h"

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
    emit leaveRoom();
    accept();
}