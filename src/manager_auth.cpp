#include "../include/manager_auth.hpp"
#include <QDate>
#include <QMap>
#include <QDebug>
#include <algorithm>

QString ManagerAuth::login(const QString &username, const QString &password, int &userId)
{
    QVector<QStringList> admins = CsvHandler::readCsv("admins.csv");
    for (const auto &row : admins)
    {
        if (row.size() >= 3 && row[1] == username && row[2] == password)
        {
            userId = row[0].toInt();
            return "Admin";
        }
    }

    QVector<QStringList> students = CsvHandler::readCsv("students.csv");
    for (const auto &row : students)
    {
        if (row.size() >= 5 && row[3] == username && row[4] == password)
        {
            userId = row[0].toInt();
            return "Student";
        }
    }

    QVector<QStringList> teachers = CsvHandler::readCsv("teachers.csv");
    for (const auto &row : teachers)
    {
        if (row.size() >= 5 && row[3] == username && row[4] == password)
        {
            userId = row[0].toInt();
            return "Teacher";
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
