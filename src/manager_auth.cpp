#include "../include/manager_auth.hpp"
#include <QDate>
#include <QMap>
#include <QDebug>
#include <algorithm>

QString ManagerAuth::login(const QString &username, const QString &password, int &userId, const QString &role)
{
    QString filename = role.toLower() + "s.csv";
    int userCol = (role == "Admin") ? 1 : 3;
    int passCol = (role == "Admin") ? 2 : 4;

    QVector<QStringList> data = CsvHandler::readCsv(filename);
    for (const auto &row : data)
    {
        if (row.size() > passCol && row[userCol].trimmed() == username && row[passCol].trimmed() == password)
        {
            userId = row[0].toInt();
            return role;
        }
    }

    return "";
}

bool ManagerAuth::changePassword(int userId, const QString &role, const QString &oldPass, const QString &newPass)
{
    QString filename;
    int passIndex = 4;

    if (role == "Student")
    {
        filename = "students.csv";
    }
    else if (role == "Teacher")
    {
        filename = "teachers.csv";
    }
    else if (role == "Admin")
    {
        filename = "admins.csv";
        passIndex = 2;
    }
    else
    {
        return false;
    }

    QVector<QStringList> data = CsvHandler::readCsv(filename);
    bool found = false;

    for (auto &row : data)
    {
        if (row.size() > passIndex && row[0].toInt() == userId)
        {
            if (row[passIndex] == oldPass)
            {
                row[passIndex] = newPass;
                found = true;
            }
            else
            {
                throw Acadence::Exception("Old password does not match.");
            }
            break;
        }
    }

    if (found)
    {
        CsvHandler::writeCsv(filename, data);
        return true;
    }
    throw Acadence::Exception("User not found.");
}

QString ManagerAuth::getDashboardStats(int userId, QString role)
{
    if (role == "Student")
    {
        // Need forward declaration or include for getStudent - will call through main appmanager
        // This is a workaround - the appmanager will handle the call
        return "";
    }
    else if (role == "Teacher")
    {
        return "Active Courses: 0";
    }
    return "System Active";
}
