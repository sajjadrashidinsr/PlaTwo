#ifndef SIGNUPPAGE_H
#define SIGNUPPAGE_H

#include <QWidget>
#include <QAction>
#include "storage_manager.h"
#include "auth_manager.h"
#include "user.h"

namespace Ui {
class signuppage;
}

class signuppage : public QWidget
{
    Q_OBJECT

public:
    explicit signuppage(storage_manager* storage,QWidget *parent = nullptr);
    ~signuppage();

private:
    Ui::signuppage *ui;
    storage_manager* storageManager;

    QAction *passwordAction;
    QAction *confirmpasswordAction;

    bool passwordVisible = false;
    bool confirmpasswordVisible = false;

signals:
    void loginClicked();

    void signUpSuccessful();

private slots:
    void updateCreateButton();

    void on_btnSubmitSignUp_clicked();

};

#endif // SIGNUPPAGE_H
