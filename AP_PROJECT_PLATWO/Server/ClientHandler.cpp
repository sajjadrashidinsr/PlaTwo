#include "ClientHandler.h"

#include <string>

using namespace std;

ClientHandler::ClientHandler(
    SOCKET clientSocket,
    RequestProcessor* processor)
{
    this->clientSocket = clientSocket;
    this->processor = processor;
}

void ClientHandler::run()
{
    while (true)
    {
        char buffer[1025];

        int bytes = recv(
            clientSocket,
            buffer,
            1024,
            0
        );

        if (bytes <= 0)
            break;

        buffer[bytes] = '\0';

        string request(buffer);

        string response =
            processor->process(request);

        send(
            clientSocket,
            response.c_str(),
            static_cast<int>(response.size()),
            0
        );
    }

    closesocket(clientSocket);
}