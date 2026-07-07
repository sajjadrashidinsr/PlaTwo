#include "Database.h"

Database::Database()
{
    database = nullptr;
}

Database::~Database()
{
    close();
}

bool Database::open(const string& fileName)
{
    return sqlite3_open(
        fileName.c_str(),
        &database
    ) == SQLITE_OK;
}

void Database::close()
{
    if (database != nullptr)
    {
        sqlite3_close(database);
        database = nullptr;
    }
}

bool Database::createTable()
{
    string sql =
        "CREATE TABLE IF NOT EXISTS users("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT,"
        "username TEXT UNIQUE,"
        "password TEXT,"
        "phone TEXT UNIQUE,"
        "email TEXT"
        ");";

    char* error = nullptr;

    int result =
        sqlite3_exec(
            database,
            sql.c_str(),
            nullptr,
            nullptr,
            &error
        );

    if (error != nullptr)
    {
        sqlite3_free(error);
    }

    return result == SQLITE_OK;
}

User Database::getUserByUsername(const string& username)
{
    std::lock_guard<std::mutex> lock(mutex);

    User user;

    sqlite3_stmt* stmt;

    string sql =
        "SELECT id, name, username, password, phone, email "
        "FROM users "
        "WHERE username = ?;";

    if (sqlite3_prepare_v2(
        database,
        sql.c_str(),
        -1,
        &stmt,
        nullptr) != SQLITE_OK)
    {
        return user;
    }

    sqlite3_bind_text(
        stmt,
        1,
        username.c_str(),
        -1,
        SQLITE_TRANSIENT
    );

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        user.setId(sqlite3_column_int(stmt, 0));

        user.setName(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));

        user.setUsername(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));

        user.setPasswordHash(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));

        user.setPhone(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));

        user.setEmail(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)));
    }

    sqlite3_finalize(stmt);

    return user;
}

bool Database::usernameExists(const string& username)
{
    std::lock_guard<std::mutex> lock(mutex);

    sqlite3_stmt* stmt;

    string sql =
        "SELECT COUNT(*) FROM users WHERE username=?;";

    sqlite3_prepare_v2(
        database,
        sql.c_str(),
        -1,
        &stmt,
        nullptr
    );

    sqlite3_bind_text(
        stmt,
        1,
        username.c_str(),
        -1,
        SQLITE_STATIC
    );

    bool exists = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);

    return exists;
}

bool Database::phoneExists(const string& phone)
{
    std::lock_guard<std::mutex> lock(mutex);

    sqlite3_stmt* stmt;

    string sql =
        "SELECT COUNT(*) FROM users WHERE phone=?;";

    sqlite3_prepare_v2(
        database,
        sql.c_str(),
        -1,
        &stmt,
        nullptr
    );

    sqlite3_bind_text(
        stmt,
        1,
        phone.c_str(),
        -1,
        SQLITE_STATIC
    );

    bool exists = false;

    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        exists = sqlite3_column_int(stmt, 0) > 0;
    }

    sqlite3_finalize(stmt);

    return exists;
}

bool Database::addUser(const User& user)
{
    std::lock_guard<std::mutex> lock(mutex);

    sqlite3_stmt* stmt;

    string sql =
        "INSERT INTO users(name,username,password,phone,email)"
        "VALUES(?,?,?,?,?);";

    sqlite3_prepare_v2(
        database,
        sql.c_str(),
        -1,
        &stmt,
        nullptr
    );

    sqlite3_bind_text(
        stmt,
        1,
        user.getName().c_str(),
        -1,
        SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt,
        2,
        user.getUsername().c_str(),
        -1,
        SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt,
        3,
        user.getPasswordHash().c_str(),
        -1,
        SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt,
        4,
        user.getPhone().c_str(),
        -1,
        SQLITE_STATIC
    );

    sqlite3_bind_text(
        stmt,
        5,
        user.getEmail().c_str(),
        -1,
        SQLITE_STATIC
    );

    bool success =
        sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);

    return success;
}


    bool Database::login(
        const string & username,
        const string & passwordHash,
        User & user)
    {
        std::lock_guard<std::mutex> lock(mutex);

        sqlite3_stmt* stmt;

        string sql =
            "SELECT id,name,username,password,phone,email "
            "FROM users "
            "WHERE username=? AND password=?;";

        sqlite3_prepare_v2(
            database,
            sql.c_str(),
            -1,
            &stmt,
            nullptr
        );

        sqlite3_bind_text(
            stmt,
            1,
            username.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_text(
            stmt,
            2,
            passwordHash.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        bool success = false;

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            user.setId(
                sqlite3_column_int(stmt, 0));

            user.setName(
                (char*)sqlite3_column_text(stmt, 1));

            user.setUsername(
                (char*)sqlite3_column_text(stmt, 2));

            user.setPasswordHash(
                (char*)sqlite3_column_text(stmt, 3));

            user.setPhone(
                (char*)sqlite3_column_text(stmt, 4));

            user.setEmail(
                (char*)sqlite3_column_text(stmt, 5));

            success = true;
        }

        sqlite3_finalize(stmt);

        return success;
    }

    bool Database::updatePassword(
        const string& phone,
        const string& passwordHash)
    {
        std::lock_guard<std::mutex> lock(mutex);

        sqlite3_stmt* stmt;

        string sql =
            "UPDATE users "
            "SET password=? "
            "WHERE phone=?;";

        sqlite3_prepare_v2(
            database,
            sql.c_str(),
            -1,
            &stmt,
            nullptr
        );

        sqlite3_bind_text(
            stmt,
            1,
            passwordHash.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_text(
            stmt,
            2,
            phone.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        bool success =
            sqlite3_step(stmt) == SQLITE_DONE;

        sqlite3_finalize(stmt);

        return success;
    }