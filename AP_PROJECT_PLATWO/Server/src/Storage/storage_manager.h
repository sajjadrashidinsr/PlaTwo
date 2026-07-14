#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "user.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QVariant>
#include <QDebug>
#include <QMutex>
#include <QThreadStorage>

class storage_manager
{
private:
    QString databasePath;
    QMutex dbMutex;

    // هر ترد اتصال دیتابیس مخصوص خود را دارد
    QThreadStorage<QSqlDatabase*> threadDb;

    QSqlDatabase getThreadDatabase();
    void initDatabase(QSqlDatabase& db);

public:
    storage_manager();
    ~storage_manager();

    bool registeruser(const user& user);
    user* getuser(const QString& username);
    bool updateuser(const user& user);
    bool addGameRecord(const QString& username, const GameRecord& record);

    // تابع برای خالی کردن دیتابیس (در صورت نیاز)
    bool clearAllData();
};

#endif // STORAGE_MANAGER_H