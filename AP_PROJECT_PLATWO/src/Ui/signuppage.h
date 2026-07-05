#ifndef SIGNUPPAGE_H
#define SIGNUPPAGE_H

#include <QWidget>
#include <QAction>

namespace Ui {
class signuppage;
}

class signuppage : public QWidget
{
    Q_OBJECT

public:
    explicit signuppage(QWidget *parent = nullptr);
    ~signuppage();

private:
    Ui::signuppage *ui;

    QAction *passwordAction;
    QAction *confirmpasswordAction;

    bool passwordVisible = false;
    bool confirmpasswordVisible = false;

signals:
    void loginClicked();

private slots:
    void updateCreateButton();

};

#endif // SIGNUPPAGE_H
