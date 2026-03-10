#ifndef MANAGER_TASKS_HPP
#define MANAGER_TASKS_HPP

#include <QString>
#include <QVector>
#include "appmanager.hpp"

class ManagerTasks
{
public:
    static QVector<Task> getTasks(int userId);
    static void addTask(int userId, const QString &description);
    static void completeTask(int taskId, bool status);
    static void deleteTask(int taskId);
    static void deleteCompletedTasks(int userId);
};

#endif // MANAGER_TASKS_HPP
