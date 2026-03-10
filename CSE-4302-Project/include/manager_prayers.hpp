#ifndef MANAGER_PRAYERS_HPP
#define MANAGER_PRAYERS_HPP

#include <QString>
#include "routine.hpp"
#include "exceptions.hpp"
#include "habit.hpp"

class ManagerPrayers
{
public:
    static DailyPrayerStatus getDailyPrayers(int userId, QString date);
    static void updateDailyPrayer(int userId, QString date, QString prayer, bool status);
};

#endif // MANAGER_PRAYERS_HPP
