#ifndef MANAGER_AUTH_HPP
#define MANAGER_AUTH_HPP

#include <QString>
#include "csvhandler.hpp"
#include "exceptions.hpp"

class ManagerAuth
{
public:
    static QString login(const QString &username, const QString &password, int &userId);
    static bool changePassword(int userId, const QString &role, const QString &oldPass, const QString &newPass);
    static QString getDashboardStats(int userId, QString role);
};

#endif // MANAGER_AUTH_HPP
