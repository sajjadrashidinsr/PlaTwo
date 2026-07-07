#pragma once

#include <string>

using namespace std;

class User
{
private:

    int id;

    string name;

    string username;

    string passwordHash;

    string phone;

    string email;

public:

    User();

    User(
        int id,
        const string& name,
        const string& username,
        const string& passwordHash,
        const string& phone,
        const string& email
    );

    void setId(int id);

    void setName(const string& name);

    void setUsername(const string& username);

    void setPasswordHash(const string& passwordHash);

    void setPhone(const string& phone);

    void setEmail(const string& email);

    int getId() const;

    string getName() const;

    string getUsername() const;

    string getPasswordHash() const;

    string getPhone() const;

    string getEmail() const;
};