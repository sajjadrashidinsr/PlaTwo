#include "Server.h"

Server::Server()
{
    serverSocket = INVALID_SOCKET;
    clientSocket = INVALID_SOCKET;
}

bool Server::initialize()
{
    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return false;

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET)
        return false;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket,
        (sockaddr*)&serverAddress,
        sizeof(serverAddress)) == SOCKET_ERROR)
        return false;

    if (listen(serverSocket, 5) == SOCKET_ERROR)
        return false;

    cout << "Server Started...\n";

    return true;
}

void Server::start()
{
    cout << "Waiting For Client...\n";

    clientSocket = accept(serverSocket, nullptr, nullptr);

    if (clientSocket == INVALID_SOCKET)
    {
        cout << "Accept Failed\n";
        return;
    }

    cout << "Client Connected\n";

    receiveMessage();
}

void Server::receiveMessage()
{
    char buffer[1024];

    int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytes > 0)
    {
        buffer[bytes] = '\0';

        cout << "Message : " << buffer << endl;

        string response = "Hello Client";

        send(clientSocket,
            response.c_str(),
            response.size(),
            0);
    }
}

Server::~Server()
{
    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();
}