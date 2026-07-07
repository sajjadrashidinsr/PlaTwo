#pragma once

#include "Database.h"
#include "User.h"

using namespace std;

class UserService
{
private:

    Database* database;

public:

    UserService(Database* database);

    bool signup(User& user, string& message);

    bool login(User& user, string& message);

    bool forgotPassword(
        const string& phone,
        const string& password,
        string& message
    );
};