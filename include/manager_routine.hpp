#ifndef MANAGER_ROUTINE_HPP
#define MANAGER_ROUTINE_HPP

#include <QString>
#include <QVector>
#include <QDate>
#include "routine.hpp"
#include "appmanager.hpp"

class ManagerRoutine
{
public:
    static QVector<RoutineSession> getRoutineForDay(QString day, int semester = -1);
    static void addRoutineItem(QString day, int serial, QString code, QString name, QString room, QString instructor, int semester);
    static QVector<RoutineAdjustment> getRoutineAdjustments();
    static void addRoutineAdjustment(const RoutineAdjustment &adj);
    static QVector<RoutineSession> getEffectiveRoutine(QDate date, int semester = -1);
    static QVector<RescheduleOption> getRescheduleOptions(QDate originDate, int originSerial, int semester, QString originCode, QString originRoom, QString instructorName);
};

#endif // MANAGER_ROUTINE_HPP
