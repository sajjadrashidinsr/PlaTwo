#pragma once

#include <string>
#include "json.hpp"
#include "User.h"

using namespace std;
using json = nlohmann::json;

class Packet
{
public:

    static string createLoginRequest(const User& user);

    static string createSignupRequest(const User& user);

    static json parse(const string& message);
};

