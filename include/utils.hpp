#ifndef UTILS_HPP
#define UTILS_HPP

#include <QString>

class Utils
{
public:
    static QString validatePassword(const QString &password);
    static QString validateUsername(const QString &username);
};

#endif // UTILS_HPP