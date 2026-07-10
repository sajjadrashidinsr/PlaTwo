#ifndef AUTH_MANAGER_H
#define AUTH_MANAGER_H

#include <QString>
#include <QRegularExpression>
#include <QCryptographicHash>

class AuthManager {

public:

    static bool validateEmail(const QString& email);

    static bool validatePhone(const QString& phone);

    static QString hashPassword(const QString& password);

    static bool verifyPhoneForRecovery(const QString& userPhone, const QString& inputPhone);

};

#endif // AUTH_MANAGER_H
