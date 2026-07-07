#pragma once

#include <string>

using namespace std;

class Validator
{
public:

    static bool isValidPhone(const string& phone);

    static bool isValidEmail(const string& email);

    static bool isStrongPassword(const string& password);
};
