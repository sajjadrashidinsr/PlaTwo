#ifndef WAITINGROOMDIALOG_H
#define WAITINGROOMDIALOG_H

#include <QDialog>
#include "../Models/Room.h"

namespace Ui {
class WaitingRoomDialog;
}

class WaitingRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaitingRoomDialog(bool isHost, const Room& room, QWidget *parent = nullptr);
    ~WaitingRoomDialog();

signals:
    void leaveRoom();

private slots:
    void onCopyAddressClicked();
    void onLeaveClicked();

private:
    void updateInfo();

    Ui::WaitingRoomDialog *ui;
    bool m_isHost;
    Room m_room;
};

#endif // WAITINGROOMDIALOG_H