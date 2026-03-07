#include "../include/manager_tasks.hpp"
#include "../include/csvhandler.hpp"
#include <algorithm>

QVector<Task> ManagerTasks::getTasks(int userId)
{
    QVector<Task> tasks;
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 4 && row[1].toInt() == userId)
            tasks.append(Task(row[0].toInt(), row[2], (row[3] == "1")));
    }
    return tasks;
}

void ManagerTasks::addTask(int userId, const QString &description)
{
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());
    CsvHandler::appendCsv("tasks.csv", {QString::number(maxId + 1), QString::number(userId), description, "0"});
}

void ManagerTasks::completeTask(int taskId, bool status)
{
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    for (auto &row : data)
    {
        if (row.size() >= 4 && row[0].toInt() == taskId)
            row[3] = status ? "1" : "0";
    }
    CsvHandler::writeCsv("tasks.csv", data);
}

void ManagerTasks::deleteTask(int taskId)
{
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    QVector<QStringList> newData;
    for (const auto &row : data)
    {
        if (row.size() > 0 && row[0].toInt() != taskId)
            newData.append(row);
    }
    CsvHandler::writeCsv("tasks.csv", newData);
}

void ManagerTasks::deleteCompletedTasks(int userId)
{
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    QVector<QStringList> newData;
    for (const auto &row : data)
    {
        if (row.size() < 4 || row[1].toInt() != userId || row[3] != "1")
            newData.append(row);
    }
    CsvHandler::writeCsv("tasks.csv", newData);
}
