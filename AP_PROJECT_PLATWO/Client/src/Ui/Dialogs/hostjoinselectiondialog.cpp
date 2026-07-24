#include "hostjoinselectiondialog.h"
#include "ui_hostjoinselectiondialog.h"

HostJoinSelectionDialog::HostJoinSelectionDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::HostJoinSelectionDialog)
{
    ui->setupUi(this);
    connect(ui->continueButton, &QPushButton::clicked, this, &HostJoinSelectionDialog::onContinueClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

HostJoinSelectionDialog::~HostJoinSelectionDialog()
{
    delete ui;
}

void HostJoinSelectionDialog::onContinueClicked()
{
    m_isHostSelected = ui->hostRadio->isChecked();
    if (m_isHostSelected)
        emit hostSelected();
    else
        emit joinSelected();
    accept();
}