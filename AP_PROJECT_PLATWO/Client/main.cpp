#include "Client.h"

int main()
{
    Client client;

    if (!client.connectServer())
    {
        cout << "Connection Failed\n";
        return 0;
    }

    client.sendMessage();

    return 0;
}