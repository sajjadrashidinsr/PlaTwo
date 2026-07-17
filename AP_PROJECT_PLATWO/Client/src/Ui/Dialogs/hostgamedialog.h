#ifndef HOSTGAMEDIALOG_H
#define HOSTGAMEDIALOG_H

#include <QDialog>
#include "../Models/GameSettings.h"

namespace Ui {
class HostGameDialog;
}

class HostGameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HostGameDialog(QWidget *parent = nullptr);
    ~HostGameDialog();

signals:
    void createRoom(const QString& roomName,
                    quint16 port,
                    const GameSettings& settings,
                    const QString& password);

private slots:
    void onTimeControlToggled(bool checked);
    void onPortEditingFinished();
    void onCreateClicked();

private:
    Ui::HostGameDialog *ui;
    quint16 m_lastValidPort = 1234;
};

#endif // HOSTGAMEDIALOG_H