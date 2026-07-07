#include "Server.h"
#include "Packet.h"


Server::Server()
    : service(&database)
{
    serverSocket = INVALID_SOCKET;
    clientSocket = INVALID_SOCKET;

    processor = nullptr;
}

bool Server::initialize()
{
    if (!database.open("users.db"))
        return false;

    database.createTable();

    processor = new RequestProcessor(&service);

    WSADATA wsa;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return false;

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET)
        return false;

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (::bind(serverSocket,
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
    while (true)
    {
        SOCKET clientSocket =
            accept(
                serverSocket,
                nullptr,
                nullptr
            );

        if (clientSocket == INVALID_SOCKET)
            continue;

        thread clientThread(
            [this, clientSocket]()
            {
                ClientHandler handler(
                    clientSocket,
                    processor
                );

                handler.run();
            }
        );

        clientThread.detach();
    }
}

void Server::receiveMessage()
{
    char buffer[1024];

    int bytes = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytes > 0)
    {
        buffer[bytes] = '\0';

        string request(buffer);

        string response =
            processor->process(request);

        send(
            clientSocket,
            response.c_str(),
            response.size(),
            0
        );

     }
}

Server::~Server()
{
    delete processor;

    if (clientSocket != INVALID_SOCKET)
        closesocket(clientSocket);

    if (serverSocket != INVALID_SOCKET)
        closesocket(serverSocket);

    WSACleanup();
}