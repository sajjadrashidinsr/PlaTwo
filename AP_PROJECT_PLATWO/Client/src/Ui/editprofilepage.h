#ifndef EDITPROFILEPAGE_H
#define EDITPROFILEPAGE_H

#include <QWidget>
#include <QPointer>
#include "user.h"

class ClientManager;

namespace Ui {
class EditProfilePage;
}

class EditProfilePage : public QWidget
{
    Q_OBJECT

public:
    explicit EditProfilePage(user* currentUser, QPointer<ClientManager> client, QWidget *parent = nullptr);
    ~EditProfilePage();

    void setUser(user* u);
    void loadUserData();

signals:
    void backToMenu();
    void profileUpdated();

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onUpdateUserResponse(bool success, const QString& message);

private:
    Ui::EditProfilePage *ui;
    user* currentUser;
    QPointer<ClientManager> clientManager;

    bool validateInputs();
};

#endif // EDITPROFILEPAGE_H