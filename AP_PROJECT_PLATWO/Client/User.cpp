#include "User.h"

User::User()
{
    id = 0;
}

User::User(
    int id,
    const string& name,
    const string& username,
    const string& passwordHash,
    const string& phone,
    const string& email)
{
    this->id = id;
    this->name = name;
    this->username = username;
    this->passwordHash = passwordHash;
    this->phone = phone;
    this->email = email;
}

void User::setId(int id)
{
    this->id = id;
}

void User::setName(const string& name)
{
    this->name = name;
}

void User::setUsername(const string& username)
{
    this->username = username;
}

void User::setPasswordHash(const string& passwordHash)
{
    this->passwordHash = passwordHash;
}

void User::setPhone(const string& phone)
{
    this->phone = phone;
}

void User::setEmail(const string& email)
{
    this->email = email;
}

int User::getId() const
{
    return id;
}

string User::getName() const
{
    return name;
}

string User::getUsername() const
{
    return username;
}

string User::getPasswordHash() const
{
    return passwordHash;
}

string User::getPhone() const
{
    return phone;
}

string User::getEmail() const
{
    return email;
}