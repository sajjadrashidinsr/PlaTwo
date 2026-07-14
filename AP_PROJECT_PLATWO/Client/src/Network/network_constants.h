#ifndef NETWORK_CONSTANTS_H
#define NETWORK_CONSTANTS_H

#include <QString>

namespace NetworkConstants {
    const quint16 DEFAULT_PORT = 1234;
    const QString SERVER_ADDRESS = "127.0.0.1";

    enum MessageType {
        // Client -> Server
        MSG_REGISTER = 1,
        MSG_LOGIN = 2,
        MSG_FORGOT_PASSWORD = 3,
        MSG_CHANGE_PASSWORD = 4,
        MSG_GET_USER = 5,
        MSG_UPDATE_USER = 6,

        // Server -> Client
        MSG_SUCCESS = 100,
        MSG_ERROR = 101,
        MSG_REGISTER_SUCCESS = 103,
        MSG_PASSWORD_CHANGED = 104
    };
}

#endif // NETWORK_CONSTANTS_H