#include "Server.h"

int main()
{
    Server server;

    if (!server.initialize())
        return 0;

    server.start();

    return 0;
}