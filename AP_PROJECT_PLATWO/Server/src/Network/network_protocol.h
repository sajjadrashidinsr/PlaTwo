#ifndef NETWORK_PROTOCOL_H
#define NETWORK_PROTOCOL_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include "user.h"
#include "../Models/GameSettings.h"
#include "../Models/room.h"

class NetworkProtocol {
public:
    // ---------- User ----------
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

    // ---------- GameSettings ----------
    static inline QJsonObject gameSettingsToJson(const GameSettings& settings) {
        QJsonObject obj;
        obj["boardSize"] = settings.boardSize;
        obj["timed"] = settings.timed;
        obj["minutes"] = settings.minutes;
        obj["seconds"] = settings.seconds;
        return obj;
    }

    static inline GameSettings gameSettingsFromJson(const QJsonObject& obj) {
        GameSettings settings;
        settings.boardSize = obj["boardSize"].toInt(3);
        settings.timed = obj["timed"].toBool(false);
        settings.minutes = obj["minutes"].toInt(0);
        settings.seconds = obj["seconds"].toInt(0);
        return settings;
    }

    // ---------- Room ----------
    static inline QJsonObject roomToJson(const Room& room) {
        QJsonObject obj;
        obj["roomName"] = room.roomName;
        obj["port"] = static_cast<int>(room.port);
        obj["hostUsername"] = room.hostUsername;
        obj["guestUsername"] = room.guestUsername;
        obj["password"] = room.password;
        obj["gameSettings"] = gameSettingsToJson(room.gameSettings);
        obj["status"] = static_cast<int>(room.status);
        return obj;
    }

    static inline Room roomFromJson(const QJsonObject& obj) {
        Room room;
        room.roomName = obj["roomName"].toString();
        room.port = static_cast<quint16>(obj["port"].toInt(1234));
        room.hostUsername = obj["hostUsername"].toString();
        room.guestUsername = obj["guestUsername"].toString();
        room.password = obj["password"].toString();
        room.gameSettings = gameSettingsFromJson(obj["gameSettings"].toObject());
        room.status = static_cast<RoomStatus>(obj["status"].toInt(0));
        return room;
    }

    // ---------- Generic ----------
    static QString buildMessage(int type, const QJsonObject& data) {
        QJsonObject obj;
        obj["type"] = type;
        obj["data"] = data;
        QJsonDocument doc(obj);
        return QString(doc.toJson(QJsonDocument::Compact)) + "\n";
    }

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