#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H
#include "user.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QVariant>
#include <QDebug>

class storage_manager
{
private:
    QSqlDatabase db;

    void initDatabase();

public:
    storage_manager();

    ~storage_manager();

    bool registeruser(const user& user);

    user* getuser(const QString& username);

    bool updateuser(const user& user);

    bool addGameRecord(const QString& username, const GameRecord& record);

};


#endif // STORAGE_MANAGER_H
