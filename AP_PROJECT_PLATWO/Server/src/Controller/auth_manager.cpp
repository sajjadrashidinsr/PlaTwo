#include "auth_manager.h"

bool AuthManager::validateEmail(const QString& email) {
    QRegularExpression regex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return regex.match(email).hasMatch();
}

bool AuthManager::validatePhone(const QString& phone) {
    QRegularExpression regex(R"(^(09|\+989)\d{9}$)");
    return regex.match(phone).hasMatch();
}

QString AuthManager::hashPassword(const QString& password) {
    if (password.length() < 8) return "";
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hash.toHex());
}

bool AuthManager::verifyPhoneForRecovery(const QString& userPhone, const QString& inputPhone) {
    return userPhone == inputPhone;
}