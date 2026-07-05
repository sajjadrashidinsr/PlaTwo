#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib,"ws2_32.lib")

using namespace std;

class Client
{
private:

    SOCKET clientSocket;

    sockaddr_in serverAddress;

public:

    Client();

    bool connectServer();

    void sendMessage();

    ~Client();
};