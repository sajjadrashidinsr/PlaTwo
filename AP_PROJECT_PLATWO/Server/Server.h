#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#include "Database.h"
#include "RequestProcessor.h"

#include <thread>
#include "ClientHandler.h"
#include "UserService.h"    

#pragma comment(lib,"ws2_32.lib")

using namespace std;

class Server
{
private:

    SOCKET serverSocket;
    SOCKET clientSocket;

    sockaddr_in serverAddress;

    Database database;

    UserService service;

    RequestProcessor* processor;
public:

    Server();

    bool initialize();

    void start();

    void receiveMessage();

    ~Server();
};

