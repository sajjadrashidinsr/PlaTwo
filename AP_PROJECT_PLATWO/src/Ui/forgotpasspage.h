#ifndef FORGOTPASSPAGE_H
#define FORGOTPASSPAGE_H

#include <QWidget>
#include <QAction>
#include "storage_manager.h"
#include "auth_manager.h"
#include "user.h"

namespace Ui {
class forgotpasspage;
}

class forgotpasspage : public QWidget
{
    Q_OBJECT

public:
    explicit forgotpasspage(storage_manager* storage,QWidget *parent = nullptr);
    ~forgotpasspage();

private:
    Ui::forgotpasspage *ui;
    storage_manager* Storage;

    QAction* userAction;

    QAction *eyeNewPass;
    QAction *eyeConfirmPass;

    bool newPassVisible = false;
    bool confirmPassVisible = false;

signals:

    void backToLoginClicked();

    void passwordChanged();


private slots:

    void toggleNewPassword();
    void toggleConfirmPassword();

    void checkVerifyButton();
    void checkChangeButton();

    void on_btnChangePassword_clicked();
};

#endif // FORGOTPASSPAGE_H
