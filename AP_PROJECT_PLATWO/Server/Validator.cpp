#include "Validator.h"

#include <regex>

bool Validator::isValidPhone(const string& phone)
{
    regex pattern("^09\\d{9}$");

    return regex_match(phone, pattern);
}

bool Validator::isValidEmail(const string& email)
{
    regex pattern(
        R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)"
    );

    return regex_match(email, pattern);
}

bool Validator::isStrongPassword(const string& password)
{
    if (password.length() < 8)
        return false;

    bool upper = false;
    bool lower = false;
    bool digit = false;
    bool special = false;

    for (char c : password)
    {
        if (isupper(c))
            upper = true;

        else if (islower(c))
            lower = true;

        else if (isdigit(c))
            digit = true;

        else
            special = true;
    }

    return upper && lower && digit && special;
}