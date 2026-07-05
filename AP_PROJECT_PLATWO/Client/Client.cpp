#include "Client.h"

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
    string message;

    cout << "Message : ";
    getline(cin >> ws, message);

    send(clientSocket,
        message.c_str(),
        message.size(),
        0);

    char buffer[1024];

    int bytes = recv(clientSocket,
        buffer,
        sizeof(buffer),
        0);

    if (bytes > 0)
    {
        buffer[bytes] = '\0';

        cout << "Server : " << buffer << endl;
    }
}

Client::~Client()
{
    closesocket(clientSocket);

    WSACleanup();
}