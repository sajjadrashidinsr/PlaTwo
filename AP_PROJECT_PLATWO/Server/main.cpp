#include "Database.h"
#include "Server.h"

int main()
{
    Database database;

    if (!database.open("users.db"))
    {
        cout << "Database Error\n";
        return 0;
    }

    database.createTable();

    Server server;

    if (!server.initialize())
        return 0;

    server.start();

    return 0;
}