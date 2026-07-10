#include "storage_manager.h"

storage_manager::storage_manager() {
    initDatabase();
}

void storage_manager::initDatabase() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("platwo_database.sqlite");

    if (!db.open()) {
        qDebug() << "Error: Connection with database failed";
        return;
    }

    // ایجاد جدول کاربران
    QSqlQuery query;
    query.exec(R"(
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
        )");

    // ایجاد جدول تاریخچه بازی‌ها
    query.exec(R"(
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
        )");
}

storage_manager::~storage_manager() {
    if (db.isOpen()) {
        db.close();
    }
}

bool storage_manager::registeruser(const user& user) {
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, name, phone, email, passwordHash) "
                  "VALUES (:username, :name, :phone, :email, :passwordHash)");
    query.bindValue(":username", user.username);
    query.bindValue(":name", user.name);
    query.bindValue(":phone", user.phone);
    query.bindValue(":email", user.email);
    query.bindValue(":passwordHash", user.passwordHash);

    return query.exec(); // اگر نام کاربری تکراری باشد، false برمی‌گرداند
}

user* storage_manager::getuser(const QString& username) {
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec() || !query.next()) {
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

    // دریافت تاریخچه بازی‌های این کاربر
    QSqlQuery historyQuery;
    historyQuery.prepare("SELECT * FROM GameHistory WHERE username = :username");
    historyQuery.bindValue(":username", username);
    historyQuery.exec();

    while (historyQuery.next()) {
        GameRecord record;
        record.opponentUsername = historyQuery.value("opponent").toString();
        record.date = historyQuery.value("gameDate").toString();
        record.role = historyQuery.value("role").toString();
        record.winner = historyQuery.value("winner").toString();
        record.finalScore = historyQuery.value("score").toInt();
        u->history.append(record);
    }

    return u;
}

bool storage_manager::updateuser(const user& user) {
    QSqlQuery query;
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

    return query.exec();
}

bool storage_manager::addGameRecord(const QString& username, const GameRecord& record) {
    QSqlQuery query;
    query.prepare("INSERT INTO GameHistory (username, opponent, gameDate, role, winner, score) "
                  "VALUES (:username, :opponent, :date, :role, :winner, :score)");
    query.bindValue(":username", username);
    query.bindValue(":opponent", record.opponentUsername);
    query.bindValue(":date", record.date);
    query.bindValue(":role", record.role);
    query.bindValue(":winner", record.winner);
    query.bindValue(":score", record.finalScore);

    return query.exec();
}



