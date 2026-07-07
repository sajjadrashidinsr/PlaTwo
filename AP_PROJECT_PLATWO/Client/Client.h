#pragma once

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include "Console.h"

#pragma comment(lib,"ws2_32.lib")

using namespace std;

class Client
{
private:

    SOCKET clientSocket;

    sockaddr_in serverAddress;

    Console console;

public:

    Client();

    bool connectServer();

    void sendMessage();

    void run();
    
    void login();

    void signup();

    void forgotPassword();

    ~Client();
};