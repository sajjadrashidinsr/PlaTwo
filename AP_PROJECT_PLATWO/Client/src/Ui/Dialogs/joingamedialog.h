#ifndef JOINGAMEDIALOG_H
#define JOINGAMEDIALOG_H

#include <QDialog>
#include "../Models/room.h"

class ClientManager;

namespace Ui {
class JoinGameDialog;
}

class JoinGameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JoinGameDialog(QWidget *parent = nullptr);
    ~JoinGameDialog();

    void setClientManager(ClientManager* client) { m_clientManager = client; }

signals:
    void joinSuccess(const Room& room);
    void joinFailed(const QString& error);

private slots:
    void onConnectClicked();
    void onPortEditingFinished();

private:
    Ui::JoinGameDialog *ui;
    quint16 m_lastValidPort = 1234;
    ClientManager* m_clientManager = nullptr;
};

#endif // JOINGAMEDIALOG_H