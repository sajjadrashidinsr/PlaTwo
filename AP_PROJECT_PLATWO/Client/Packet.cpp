#include "Packet.h"

string Packet::createLoginRequest(const User& user)
{
    json j;

    j["type"] = "login";
    j["username"] = user.getUsername();
    j["password"] = user.getPasswordHash();

    return j.dump();
}

string Packet::createSignupRequest(const User& user)
{
    json j;

    j["type"] = "signup";
    j["name"] = user.getName();
    j["username"] = user.getUsername();
    j["password"] = user.getPasswordHash();
    j["phone"] = user.getPhone();
    j["email"] = user.getEmail();

    return j.dump();
}

json Packet::parse(const string& message)
{
    return json::parse(message);
}