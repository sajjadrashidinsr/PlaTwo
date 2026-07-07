#include "Client.h"

int main()
{
    Client client;

    if (!client.connectServer())
        return 0;

    client.run();

    return 0;
}