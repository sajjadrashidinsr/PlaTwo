#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "user.h"

class NetworkProtocol {
public:
    // تبدیل user به JSON - تعریف inline
    static inline QJsonObject userToJson(const user& u) {
        QJsonObject obj;
        obj["username"] = u.username;
        obj["name"] = u.name;
        obj["phone"] = u.phone;
        obj["email"] = u.email;
        obj["passwordHash"] = u.passwordHash;
        obj["dotsScore"] = u.dotsAndBoxesScore;
        obj["morrisScore"] = u.nineMensMorrisScore;
        obj["fanoronaScore"] = u.fanoronaScore;

        QJsonArray historyArray;
        for (const auto& record : u.history) {
            QJsonObject recordObj;
            recordObj["opponent"] = record.opponentUsername;
            recordObj["date"] = record.date;
            recordObj["role"] = record.role;
            recordObj["winner"] = record.winner;
            recordObj["score"] = record.finalScore;
            historyArray.append(recordObj);
        }
        obj["history"] = historyArray;

        return obj;
    }

    // تبدیل JSON به user - تعریف inline
    static inline user userFromJson(const QJsonObject& obj) {
        user u;
        u.username = obj["username"].toString();
        u.name = obj["name"].toString();
        u.phone = obj["phone"].toString();
        u.email = obj["email"].toString();
        u.passwordHash = obj["passwordHash"].toString();
        u.dotsAndBoxesScore = obj["dotsScore"].toInt();
        u.nineMensMorrisScore = obj["morrisScore"].toInt();
        u.fanoronaScore = obj["fanoronaScore"].toInt();

        QJsonArray historyArray = obj["history"].toArray();
        for (const auto& value : historyArray) {
            QJsonObject recordObj = value.toObject();
            GameRecord record;
            record.opponentUsername = recordObj["opponent"].toString();
            record.date = recordObj["date"].toString();
            record.role = recordObj["role"].toString();
            record.winner = recordObj["winner"].toString();
            record.finalScore = recordObj["score"].toInt();
            u.history.append(record);
        }

        return u;
    }

    // ساخت پیام با داده JSON
    static QString buildMessage(int type, const QJsonObject& data) {
        QJsonObject obj;
        obj["type"] = type;
        obj["data"] = data;
        QJsonDocument doc(obj);
        return QString(doc.toJson(QJsonDocument::Compact)) + "\n";
    }

    // تجزیه پیام دریافتی
    static bool parseMessage(const QString& message, int& type, QJsonObject& data) {
        QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
        if (!doc.isObject()) {
            return false;
        }

        QJsonObject obj = doc.object();
        if (!obj.contains("type")) {
            return false;
        }

        type = obj["type"].toInt();
        if (obj.contains("data")) {
            data = obj["data"].toObject();
        } else {
            data = QJsonObject();
        }

        return true;
    }
};

#endif // NETWORK_PROTOCOL_H