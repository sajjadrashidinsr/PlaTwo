#ifndef HOSTJOINSELECTIONDIALOG_H
#define HOSTJOINSELECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class HostJoinSelectionDialog;
}

class HostJoinSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HostJoinSelectionDialog(QWidget *parent = nullptr);
    ~HostJoinSelectionDialog();

signals:
    void hostSelected();
    void joinSelected();

private slots:
    void onContinueClicked();

private:
    Ui::HostJoinSelectionDialog *ui;
};

#endif // HOSTJOINSELECTIONDIALOG_H