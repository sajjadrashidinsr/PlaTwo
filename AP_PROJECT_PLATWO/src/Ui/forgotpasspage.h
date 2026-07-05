#ifndef FORGOTPASSPAGE_H
#define FORGOTPASSPAGE_H

#include <QWidget>
#include <QAction>

namespace Ui {
class forgotpasspage;
}

class forgotpasspage : public QWidget
{
    Q_OBJECT

public:
    explicit forgotpasspage(QWidget *parent = nullptr);
    ~forgotpasspage();

private:
    Ui::forgotpasspage *ui;


    QAction* userAction;

    QAction *eyeNewPass;
    QAction *eyeConfirmPass;

    bool newPassVisible = false;
    bool confirmPassVisible = false;

signals:

    void backToLoginClicked();



private slots:

    void toggleNewPassword();
    void toggleConfirmPassword();

    void checkVerifyButton();
    void checkChangeButton();

};

#endif // FORGOTPASSPAGE_H
