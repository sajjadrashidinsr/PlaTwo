#pragma once

#include "User.h"

using namespace std;

class Console
{
public:

    int showMenu();

    User getSignupInformation();

    User getLoginInformation();

    string getPhone();

    string getNewPassword();

    void showMessage(const string& message);
};

