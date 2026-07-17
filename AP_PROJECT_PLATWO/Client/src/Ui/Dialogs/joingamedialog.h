#ifndef JOINGAMEDIALOG_H
#define JOINGAMEDIALOG_H

#include <QDialog>

namespace Ui {
class JoinGameDialog;
}

class JoinGameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JoinGameDialog(QWidget *parent = nullptr);
    ~JoinGameDialog();

signals:
    void connectToServer(const QString& ip, quint16 port, const QString& password);

private slots:
    void onConnectClicked();
    void onPortEditingFinished();

private:
    Ui::JoinGameDialog *ui;
    quint16 m_lastValidPort = 1234;
};

#endif // JOINGAMEDIALOG_H