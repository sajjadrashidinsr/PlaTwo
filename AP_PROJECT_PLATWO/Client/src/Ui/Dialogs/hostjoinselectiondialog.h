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

    bool isHostSelected() const { return m_isHostSelected; }

signals:
    void hostSelected();
    void joinSelected();

private slots:
    void onContinueClicked();

private:
    Ui::HostJoinSelectionDialog *ui;
    bool m_isHostSelected = true;
};

#endif // HOSTJOINSELECTIONDIALOG_H