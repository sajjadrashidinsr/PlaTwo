#pragma once

#include <winsock2.h>

#include "RequestProcessor.h"

using namespace std;

class ClientHandler
{
private:

    SOCKET clientSocket;

    RequestProcessor* processor;

public:

    ClientHandler(
        SOCKET clientSocket,
        RequestProcessor* processor
    );

    void run();
};