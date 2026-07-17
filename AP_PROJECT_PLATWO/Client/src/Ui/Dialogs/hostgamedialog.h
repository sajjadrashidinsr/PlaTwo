#ifndef HOSTGAMEDIALOG_H
#define HOSTGAMEDIALOG_H

#include <QDialog>
#include "../Models/GameSettings.h"
#include "../Models/room.h"

class ClientManager;

namespace Ui {
class HostGameDialog;
}

class HostGameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HostGameDialog(QWidget *parent = nullptr);
    ~HostGameDialog();

    void setClientManager(ClientManager* client) { m_clientManager = client; }

signals:
    // Emitted when room creation succeeds or fails (for parent to react)
    void roomCreationSuccess(const Room& room);
    void roomCreationFailed(const QString& error);

private slots:
    void onTimeControlToggled(bool checked);
    void onPortEditingFinished();
    void onCreateClicked();

private:
    Ui::HostGameDialog *ui;
    quint16 m_lastValidPort = 1234;
    ClientManager* m_clientManager = nullptr;
};

#endif // HOSTGAMEDIALOG_H