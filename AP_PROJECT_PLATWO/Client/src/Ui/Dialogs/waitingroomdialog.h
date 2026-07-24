#ifndef WAITINGROOMDIALOG_H
#define WAITINGROOMDIALOG_H

#include <QDialog>
#include "../Models/Room.h"

class ClientManager;

namespace Ui {
class WaitingRoomDialog;
}

class WaitingRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaitingRoomDialog(bool isHost, const Room& room, QWidget *parent = nullptr);
    ~WaitingRoomDialog();

    void setClientManager(ClientManager* client);

signals:
    void leaveRoom();
    void startGame(int boardSize);

private slots:
    void onCopyAddressClicked();
    void onLeaveClicked();
    void onStartGameClicked();

    void onPlayerJoined(const QString& playerName);
    void onPlayerLeft(const QString& playerName);
    void onRoomError(const QString& error);
    void onGameStarted(const QJsonObject& data);

private:
    void updateInfo();

    Ui::WaitingRoomDialog *ui;
    bool m_isHost;
    Room m_room;
    ClientManager* m_clientManager = nullptr;
    int m_startedWithBoardSize = 5;
};

#endif // WAITINGROOMDIALOG_H