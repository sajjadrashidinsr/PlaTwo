#pragma once

#include <string>
#include "sqlite3.h"
#include "User.h"
#include "PasswordHasher.h"
#include <mutex>

using namespace std;

class Database
{
private:

    sqlite3* database;

    mutable mutex mutex;

public:

    Database();

    ~Database();

    bool open(const string& fileName);

    void close();

    bool createTable();

    User getUserByUsername(const string& username);

    bool addUser(const User& user);

    bool usernameExists(const string& username);

    bool phoneExists(const string& phone);
    
    bool login(
        const string& username,
        const string& passwordHash,
        User& user
    );

    bool updatePassword(
        const string& phone,
        const string& passwordHash
    );

};

