#pragma once

#include <string>

#include "json.hpp"
#include "UserService.h"

using namespace std;
using json = nlohmann::json;

class RequestProcessor
{
private:

    UserService* service;

public:

    RequestProcessor(UserService* service);

    string process(const string& request);

private:

    string processLogin(const json& request);

    string processSignup(const json& request);

    string processForgotPassword(const json& request);
};