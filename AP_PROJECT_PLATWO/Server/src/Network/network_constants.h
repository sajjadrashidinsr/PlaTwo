#ifndef NETWORK_CONSTANTS_H
#define NETWORK_CONSTANTS_H

#include <QString>

namespace NetworkConstants {
const quint16 DEFAULT_PORT = 1234;
const QString SERVER_ADDRESS = "127.0.0.1";

enum MessageType {
    // Client -> Server (Authentication)
    MSG_REGISTER = 1,
    MSG_LOGIN = 2,
    MSG_FORGOT_PASSWORD = 3,
    MSG_CHANGE_PASSWORD = 4,
    MSG_GET_USER = 5,
    MSG_UPDATE_USER = 6,

    // Client -> Server (Room Management)
    MSG_CREATE_ROOM = 10,
    MSG_JOIN_ROOM = 11,
    MSG_LEAVE_ROOM = 12,
    MSG_ROOM_INFO = 13,

    // Server -> Client (Authentication)
    MSG_SUCCESS = 100,
    MSG_ERROR = 101,
    MSG_REGISTER_SUCCESS = 103,
    MSG_PASSWORD_CHANGED = 104,

    // Server -> Client (Room Management)
    MSG_ROOM_CREATED = 200,
    MSG_ROOM_JOINED = 201,
    MSG_PLAYER_JOINED = 202,
    MSG_PLAYER_LEFT = 203,
    MSG_ROOM_ERROR = 204
};
}

#endif // NETWORK_CONSTANTS_H