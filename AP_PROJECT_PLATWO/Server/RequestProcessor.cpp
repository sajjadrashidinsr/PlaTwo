#include "RequestProcessor.h"

#include "User.h"

RequestProcessor::RequestProcessor(UserService* service)
{
    this->service = service;
}

string RequestProcessor::process(const string& request)
{
    json j = json::parse(request);

    string type = j["type"];

    if (type == "login")
    {
        return processLogin(j);
    }

    if (type == "signup")
    {
        return processSignup(j);
    }

    if (type == "forgot")
    {
        return processForgotPassword(j);
    }

    json response;

    response["status"] = "error";
    response["message"] = "Unknown Request";

    return response.dump();
}

string RequestProcessor::processLogin(const json& request)
{
    User user;

    user.setUsername(request["username"]);
    user.setPasswordHash(request["password"]);

    string message;

    bool result =
        service->login(user, message);

    json response;

    response["status"] =
        result ? "success" : "error";

    response["message"] = message;

    return response.dump();
}

string RequestProcessor::processSignup(const json& request)
{
    User user;

    user.setName(request["name"]);

    user.setUsername(request["username"]);

    user.setPasswordHash(request["password"]);

    user.setPhone(request["phone"]);

    user.setEmail(request["email"]);

    string message;

    bool result =
        service->signup(user, message);

    json response;

    response["status"] =
        result ? "success" : "error";

    response["message"] = message;

    return response.dump();
}

string RequestProcessor::processForgotPassword(
    const json& request)
{
    string phone =
        request["phone"];

    string password =
        request["password"];

    string message;

    bool result =
        service->forgotPassword(
            phone,
            password,
            message
        );

    json response;

    response["status"] =
        result ? "success" : "error";

    response["message"] = message;

    return response.dump();
}