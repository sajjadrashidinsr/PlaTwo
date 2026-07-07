#include "Client.h"
#include "Packet.h"
#include "User.h"

Client::Client()
{
    clientSocket = INVALID_SOCKET;
}

bool Client::connectServer()
{
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return false;

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (clientSocket == INVALID_SOCKET)
        return false;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);

    inet_pton(AF_INET,
        "127.0.0.1",
        &serverAddress.sin_addr);

    if (connect(clientSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress))
        == SOCKET_ERROR)
    {
        return false;
    }

    return true;
}

void Client::sendMessage()
{
    User user;

    string temp;

    cout << "Username : ";
    getline(cin >> ws, temp);
    user.setUsername(temp);

    cout << "Password : ";
    getline(cin, temp);
    user.setPasswordHash(temp);

    string packet = Packet::createLoginRequest(user);

    send(
        clientSocket,
        packet.c_str(),
        packet.size(),
        0
    );

    char buffer[1025];

    int bytes =
        recv(
            clientSocket,
            buffer,
            1024,
            0
        );

    buffer[bytes] = '\0';

    json response =
        json::parse(buffer);

    console.showMessage(
        response["message"]);
}

void Client::run()
{
    while (true)
    {
        switch (console.showMenu())
        {
        case 1:
            login();
            break;

        case 2:
        {
            signup();
            break;
        }

        case 3:
            forgotPassword();
            break;

        case 0:
            return;
        }
    }
}

void Client::login()
{
    User user = console.getLoginInformation();

    json request;

    request["type"] = "login";
    request["username"] = user.getUsername();
    request["password"] = user.getPasswordHash();

    string message = request.dump();

    send(
        clientSocket,
        message.c_str(),
        static_cast<int>(message.size()),
        0
    );

    char buffer[1025];

    int bytes = recv(
        clientSocket,
        buffer,
        1024,
        0
    );

    if (bytes <= 0)
    {
        console.showMessage("Connection Lost");
        return;
    }

    buffer[bytes] = '\0';

    json response = json::parse(buffer);

    console.showMessage(response["message"]);
}

void Client::signup()
{
    User user = console.getSignupInformation();

    json request;

    request["type"] = "signup";
    request["name"] = user.getName();
    request["username"] = user.getUsername();
    request["password"] = user.getPasswordHash();
    request["phone"] = user.getPhone();
    request["email"] = user.getEmail();

    string message = request.dump();

    send(
        clientSocket,
        message.c_str(),
        static_cast<int>(message.size()),
        0
    );

    char buffer[1025];

    int bytes = recv(
        clientSocket,
        buffer,
        1024,
        0
    );

    if (bytes <= 0)
        return;

    buffer[bytes] = '\0';

    json response = json::parse(buffer);

    console.showMessage(response["message"]);
}

void Client::forgotPassword()
{
    string phone =
        console.getPhone();

    string password =
        console.getNewPassword();

    json request;

    request["type"] = "forgot";

    request["phone"] = phone;

    request["password"] = password;

    string message =
        request.dump();

    send(
        clientSocket,
        message.c_str(),
        static_cast<int>(message.size()),
        0
    );

    char buffer[1025];

    int bytes =
        recv(
            clientSocket,
            buffer,
            1024,
            0
        );

    if (bytes <= 0)
        return;

    buffer[bytes] = '\0';

    json response =
        json::parse(buffer);

    console.showMessage(
        response["message"]);
}

Client::~Client()
{
    closesocket(clientSocket);

    WSACleanup();
}