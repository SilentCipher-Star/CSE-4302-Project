#pragma once

#include <QString>
#include <QVector>
#include <QDate>
#include <memory>
#include <vector>
#include "models.hpp"
#include "habit.hpp"

class ManagerProductivity
{
public:
    // Habits
    static std::vector<std::unique_ptr<Habit>> getHabits(int userId);
    static void addHabit(Habit *h);
    static void updateHabit(Habit *h);
    static void deleteHabit(int id);

    // Tasks
    static QVector<Task> getTasks(int userId);
    static void addTask(int userId, const QString &description);
    static void completeTask(int taskId, bool status);
    static void deleteTask(int taskId);
    static void deleteCompletedTasks(int userId);

    // Prayers
    static DailyPrayerStatus getDailyPrayers(int userId, QString date);
    static void updateDailyPrayer(int userId, QString date, QString prayer, bool status);

    // Routine
    static QVector<RoutineSession> getRoutineForDay(QString day, int semester = -1);
    static void addRoutineItem(QString day, int serial, QString code, QString name, QString room, QString instructor, int semester);
    static QVector<RoutineAdjustment> getRoutineAdjustments();
    static void addRoutineAdjustment(const RoutineAdjustment &adj);
    static QVector<RoutineSession> getEffectiveRoutine(QDate date, int semester = -1);
    static QVector<RescheduleOption> getRescheduleOptions(QDate originDate, int originSerial, int semester, QString originCode, QString originRoom, QString instructorName);
};