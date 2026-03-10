#ifndef MANAGER_HABITS_HPP
#define MANAGER_HABITS_HPP

#include <QString>
#include <QVector>
#include "habit.hpp"

class ManagerHabits
{
public:
    static QVector<Habit *> getHabits(int userId);
    static void addHabit(Habit *h);
    static void updateHabit(Habit *h);
    static void deleteHabit(int id);
};

#endif // MANAGER_HABITS_HPP
