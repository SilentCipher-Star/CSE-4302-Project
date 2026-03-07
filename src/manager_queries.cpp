#include "../include/manager_queries.hpp"
#include "../include/csvhandler.hpp"
#include <QDateTime>
#include <QMap>
#include <algorithm>

QVector<Query> ManagerQueries::getQueries(int userId, QString role)
{
    QVector<Query> list;
    QVector<QStringList> data = CsvHandler::readCsv("queries.csv");

    // Pre-load names for display
    QMap<int, QString> studentNames;
    QVector<QStringList> sData = CsvHandler::readCsv("students.csv");
    for (const auto &row : sData)
        if (row.size() >= 2)
            studentNames[row[0].toInt()] = row[1];

    QMap<int, QString> teacherNames;
    QVector<QStringList> tData = CsvHandler::readCsv("teachers.csv");
    for (const auto &row : tData)
        if (row.size() >= 2)
            teacherNames[row[0].toInt()] = row[1];

    for (const auto &row : data)
    {
        if (row.size() >= 6)
        {
            int qId = row[0].toInt();
            int sId = row[1].toInt();
            int tId = row[2].toInt();

            bool isVisible = false;
            if (role == "Admin")
                isVisible = true;
            else if (role == "Student" && sId == userId)
                isVisible = true;
            else if (role == "Teacher" && tId == userId)
                isVisible = true;

            if (isVisible)
            {
                QString sName = studentNames.value(sId, "Unknown Student");
                QString tName = teacherNames.value(tId, "Unknown Teacher");
                list.append(Query(qId, sId, tId, sName, tName, row[3], row[4], row[5]));
            }
        }
    }
    return list;
}

void ManagerQueries::addQuery(int userId, int teacherId, QString question)
{
    QVector<QStringList> data = CsvHandler::readCsv("queries.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());

    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
    CsvHandler::appendCsv("queries.csv", {QString::number(maxId + 1), QString::number(userId), QString::number(teacherId), question, "", ts});
}

void ManagerQueries::answerQuery(int queryId, QString answer)
{
    QVector<QStringList> data = CsvHandler::readCsv("queries.csv");
    for (auto &row : data)
    {
        if (row.size() >= 6 && row[0].toInt() == queryId)
        {
            row[4] = answer;
        }
    }
    CsvHandler::writeCsv("queries.csv", data);
}

QVector<QPair<int, QString>> ManagerQueries::getTeacherList()
{
    QVector<QPair<int, QString>> list;
    QVector<QStringList> data = CsvHandler::readCsv("teachers.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 2)
            list.append({row[0].toInt(), row[1]});
    }
    return list;
}
