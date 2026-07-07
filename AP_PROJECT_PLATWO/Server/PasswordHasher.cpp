#include "PasswordHasher.h"

#include <functional>

string PasswordHasher::hashPassword(const string& password)
{
    hash<string> hasher;

    return to_string(
        hasher(password)
    );
}