#include "storage_manager.h"
#include <QThread>

storage_manager::storage_manager() {
    databasePath = "platwo_database.sqlite";
    qDebug() << "[DB] Storage manager created, database path:" << databasePath;
}

storage_manager::~storage_manager() {
    qDebug() << "[DB] Storage manager destroyed";
}

QSqlDatabase storage_manager::getThreadDatabase() {
    Qt::HANDLE threadId = QThread::currentThreadId();

    // بررسی اینکه آیا برای این ترد اتصال وجود دارد
    if (!threadDb.hasLocalData()) {
        // ایجاد اتصال جدید برای این ترد
        QString connectionName = QString("thread_%1")
                                     .arg(reinterpret_cast<quint64>(threadId), 0, 16);

        QSqlDatabase* db = new QSqlDatabase();
        *db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        db->setDatabaseName(databasePath);

        if (!db->open()) {
            qDebug() << "[DB] ERROR opening database for thread:" << connectionName;
            qDebug() << "[DB] Error:" << db->lastError().text();
            threadDb.setLocalData(nullptr);
            return QSqlDatabase();
        }

        qDebug() << "[DB] Database opened for thread:" << connectionName;

        // ایجاد جداول اگر وجود ندارند (داده‌های موجود حفظ می‌شوند)
        initDatabase(*db);

        threadDb.setLocalData(db);
        return *db;
    }

    QSqlDatabase* db = threadDb.localData();
    if (!db) {
        qDebug() << "[DB] ERROR: threadDb.localData() is null";
        return QSqlDatabase();
    }

    if (!db->isOpen()) {
        qDebug() << "[DB] Database not open, reopening...";
        if (!db->open()) {
            qDebug() << "[DB] ERROR reopening database:" << db->lastError().text();
            return QSqlDatabase();
        }
    }

    return *db;
}

void storage_manager::initDatabase(QSqlDatabase& db) {
    if (!db.isOpen()) {
        qDebug() << "[DB] Cannot init database - not open";
        return;
    }

    QSqlQuery query(db);

    // ایجاد جدول users (اگر وجود داشته باشد، ایجاد نمی‌کند و داده‌ها حفظ می‌شوند)
    QString createUsers = R"(
        CREATE TABLE IF NOT EXISTS users (
            username TEXT PRIMARY KEY,
            name TEXT,
            phone TEXT,
            email TEXT,
            passwordHash TEXT,
            dotsScore INTEGER DEFAULT 0,
            morrisScore INTEGER DEFAULT 0,
            fanoronaScore INTEGER DEFAULT 0
        )
    )";

    if (!query.exec(createUsers)) {
        qDebug() << "[DB] Error creating users table:" << query.lastError().text();
    } else {
        qDebug() << "[DB] Users table ready";
    }

    // ایجاد جدول GameHistory (اگر وجود داشته باشد، ایجاد نمی‌کند و داده‌ها حفظ می‌شوند)
    QString createHistory = R"(
        CREATE TABLE IF NOT EXISTS GameHistory (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT,
            opponent TEXT,
            gameDate TEXT,
            role TEXT,
            winner TEXT,
            score INTEGER,
            FOREIGN KEY(username) REFERENCES users(username)
        )
    )";

    if (!query.exec(createHistory)) {
        qDebug() << "[DB] Error creating GameHistory table:" << query.lastError().text();
    } else {
        qDebug() << "[DB] GameHistory table ready";
    }
}

bool storage_manager::registeruser(const user& user) {
    qDebug() << "[DB] registeruser called for username:" << user.username;
    qDebug() << "[DB] Current thread ID:" << QThread::currentThreadId();

    QMutexLocker locker(&dbMutex);

    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) {
        qDebug() << "[DB] registeruser: Database not open";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, name, phone, email, passwordHash) "
                  "VALUES (:username, :name, :phone, :email, :passwordHash)");
    query.bindValue(":username", user.username);
    query.bindValue(":name", user.name);
    query.bindValue(":phone", user.phone);
    query.bindValue(":email", user.email);
    query.bindValue(":passwordHash", user.passwordHash);

    bool result = query.exec();
    if (!result) {
        qDebug() << "[DB] registeruser error:" << query.lastError().text();
        qDebug() << "[DB] Username:" << user.username;
    } else {
        qDebug() << "[DB] User registered successfully:" << user.username;
    }
    return result;
}

user* storage_manager::getuser(const QString& username) {
    qDebug() << "[DB] getuser called for username:" << username;
    qDebug() << "[DB] Current thread ID:" << QThread::currentThreadId();

    QMutexLocker locker(&dbMutex);

    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) {
        qDebug() << "[DB] getuser: Database not open";
        return nullptr;
    }

    QSqlQuery query(db);
    query.prepare("SELECT * FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "[DB] getuser query error:" << query.lastError().text();
        return nullptr;
    }

    if (!query.next()) {
        qDebug() << "[DB] User not found:" << username;
        return nullptr;
    }

    user* u = new user();
    u->username = query.value("username").toString();
    u->name = query.value("name").toString();
    u->phone = query.value("phone").toString();
    u->email = query.value("email").toString();
    u->passwordHash = query.value("passwordHash").toString();
    u->dotsAndBoxesScore = query.value("dotsScore").toInt();
    u->nineMensMorrisScore = query.value("morrisScore").toInt();
    u->fanoronaScore = query.value("fanoronaScore").toInt();

    // دریافت تاریخچه بازی‌ها
    QSqlQuery historyQuery(db);
    historyQuery.prepare("SELECT * FROM GameHistory WHERE username = :username");
    historyQuery.bindValue(":username", username);

    if (historyQuery.exec()) {
        while (historyQuery.next()) {
            GameRecord record;
            record.opponentUsername = historyQuery.value("opponent").toString();
            record.date = historyQuery.value("gameDate").toString();
            record.role = historyQuery.value("role").toString();
            record.winner = historyQuery.value("winner").toString();
            record.finalScore = historyQuery.value("score").toInt();
            u->history.append(record);
        }
    }

    qDebug() << "[DB] User retrieved:" << username;
    return u;
}

bool storage_manager::updateuser(const user& user) {
    qDebug() << "[DB] updateuser called for username:" << user.username;

    QMutexLocker locker(&dbMutex);

    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) {
        qDebug() << "[DB] updateuser: Database not open";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("UPDATE users SET name = :name, phone = :phone, email = :email, "
                  "passwordHash = :passwordHash, dotsScore = :dotsScore, "
                  "morrisScore = :morrisScore, fanoronaScore = :fanoronaScore "
                  "WHERE username = :username");
    query.bindValue(":name", user.name);
    query.bindValue(":phone", user.phone);
    query.bindValue(":email", user.email);
    query.bindValue(":passwordHash", user.passwordHash);
    query.bindValue(":dotsScore", user.dotsAndBoxesScore);
    query.bindValue(":morrisScore", user.nineMensMorrisScore);
    query.bindValue(":fanoronaScore", user.fanoronaScore);
    query.bindValue(":username", user.username);

    bool result = query.exec();
    if (!result) {
        qDebug() << "[DB] updateuser error:" << query.lastError().text();
    } else {
        qDebug() << "[DB] User updated successfully:" << user.username;
    }
    return result;
}

bool storage_manager::addGameRecord(const QString& username, const GameRecord& record) {
    QMutexLocker locker(&dbMutex);

    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) {
        qDebug() << "[DB] addGameRecord: Database not open";
        return false;
    }

    QSqlQuery query(db);
    query.prepare("INSERT INTO GameHistory (username, opponent, gameDate, role, winner, score) "
                  "VALUES (:username, :opponent, :date, :role, :winner, :score)");
    query.bindValue(":username", username);
    query.bindValue(":opponent", record.opponentUsername);
    query.bindValue(":date", record.date);
    query.bindValue(":role", record.role);
    query.bindValue(":winner", record.winner);
    query.bindValue(":score", record.finalScore);

    bool result = query.exec();
    if (!result) {
        qDebug() << "[DB] addGameRecord error:" << query.lastError().text();
    }
    return result;
}

bool storage_manager::clearAllData() {
    QMutexLocker locker(&dbMutex);

    QSqlDatabase db = getThreadDatabase();
    if (!db.isOpen()) {
        qDebug() << "[DB] clearAllData: Database not open";
        return false;
    }

    QSqlQuery query(db);

    // حذف همه رکوردها
    if (!query.exec("DELETE FROM GameHistory")) {
        qDebug() << "[DB] Error clearing GameHistory:" << query.lastError().text();
        return false;
    }

    if (!query.exec("DELETE FROM users")) {
        qDebug() << "[DB] Error clearing users:" << query.lastError().text();
        return false;
    }

    // تنظیم مجدد شمارنده
    query.exec("DELETE FROM sqlite_sequence");

    qDebug() << "[DB] All data cleared successfully";
    return true;
}