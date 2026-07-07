#include "UserService.h"

#include "Validator.h"

#include "PasswordHasher.h"

UserService::UserService(Database* database)
{
    this->database = database;
}

bool UserService::signup(
    User& user,
    string& message)
{
    if (!Validator::isValidPhone(user.getPhone()))
    {
        message = "Invalid Phone";
        return false;
    }

    if (!Validator::isValidEmail(user.getEmail()))
    {
        message = "Invalid Email";
        return false;
    }

    if (!Validator::isStrongPassword(user.getPasswordHash()))
    {
        message = "Weak Password";
        return false;
    }

    if (database->usernameExists(user.getUsername()))
    {
        message = "Username Exists";
        return false;
    }

    if (database->phoneExists(user.getPhone()))
    {
        message = "Phone Exists";
        return false;
    }

    user.setPasswordHash(
        PasswordHasher::hashPassword(
            user.getPasswordHash()
        )
    );

    if (database->addUser(user))
    {
        message = "Signup Success";
        return true;
    }

    message = "Signup Failed";

    return false;
}

bool UserService::login(
    User& user,
    string& message)
{
    user.setPasswordHash(
        PasswordHasher::hashPassword(
            user.getPasswordHash()
        )
    );

    if (database->login(
        user.getUsername(),
        user.getPasswordHash(),
        user))
    {
        message = "Login Success";
        return true;
    }

    message = "Username Or Password Is Incorrect";

    return false;
}

bool UserService::forgotPassword(
    const string& phone,
    const string& password,
    string& message)
{
    if (!database->phoneExists(phone))
    {
        message = "Phone Not Found";
        return false;
    }

    if (!Validator::isStrongPassword(password))
    {
        message = "Weak Password";
        return false;
    }

    string hash =
        PasswordHasher::hashPassword(password);

    if (database->updatePassword(phone, hash))
    {
        message = "Password Updated";
        return true;
    }

    message = "Update Failed";

    return false;
}