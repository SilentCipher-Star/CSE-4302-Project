#include "../include/habit.hpp"

Habit::Habit(int id, int sid, QString n, HabitType t, Frequency f)
    : id(id), studentId(sid), name(n), type(t), frequency(f), streak(0), lastUpdated(QDate::currentDate()), isCompleted(false) {}

bool Habit::checkReset()
{
    QDate today = QDate::currentDate();
    bool reset = false;

    if (frequency == Frequency::DAILY)
    {
        if (lastUpdated != today)
            reset = true;
    }
    else if (frequency == Frequency::WEEKLY)
    {
        if (lastUpdated.weekNumber() != today.weekNumber() || lastUpdated.year() != today.year())
            reset = true;
    }

    if (reset)
    {
        if (streak > 0)
        {
            if (frequency == Frequency::DAILY)
            {
                if (lastUpdated < today.addDays(-1))
                {
                    streak = 0;
                }
                else if (lastUpdated == today.addDays(-1) && !isCompleted)
                {
                    streak = 0;
                }
            }
            else if (frequency == Frequency::WEEKLY)
            {
                int weekDiff = today.weekNumber() - lastUpdated.weekNumber();
                if (today.year() != lastUpdated.year())
                    weekDiff += 52 * (today.year() - lastUpdated.year());

                if (weekDiff > 1)
                    streak = 0;
                else if (weekDiff == 1 && !isCompleted)
                    streak = 0;
            }
        }

        isCompleted = false;
        lastUpdated = today;
        if (auto dh = dynamic_cast<DurationHabit *>(this))
            dh->setCurrentMinutes(0.0);
        if (auto ch = dynamic_cast<CountHabit *>(this))
            ch->setCurrentCount(0);
        return true;
    }
    return false;
}

void Habit::markComplete()
{
    if (!isCompleted)
    {
        isCompleted = true;
        streak++;
    }
}

QString Habit::getFrequencyString() const { return (frequency == Frequency::DAILY) ? "Daily" : "Weekly"; }
QString Habit::getTypeString() const
{
    switch (type)
    {
    case HabitType::DURATION:
        return "Duration";
    case HabitType::COUNT:
        return "Count";
    case HabitType::WORKOUT:
        return "Workout";
    default:
        return "Unknown";
    }
}

DurationHabit::DurationHabit(int id, int sid, QString n, Frequency f, int target)
    : Habit(id, sid, n, HabitType::DURATION, f), targetMinutes(target), currentMinutes(0.0) {}

QString DurationHabit::getProgressString() const
{
    int totalSeconds = static_cast<int>(currentMinutes * 60);
    int mins = totalSeconds / 60;
    int secs = totalSeconds % 60;
    return QString("%1:%2/%3 mins").arg(mins).arg(secs, 2, 10, QChar('0')).arg(targetMinutes);
}

QString DurationHabit::serializeValue() const
{
    return QString::number(currentMinutes, 'f', 4);
}

void DurationHabit::deserializeValue(const QString &val)
{
    currentMinutes = val.toDouble();
}

CountHabit::CountHabit(int id, int sid, QString n, Frequency f, int target, QString u)
    : Habit(id, sid, n, HabitType::COUNT, f), targetCount(target), currentCount(0), unit(u) {}

QString CountHabit::getProgressString() const
{
    return QString("%1/%2 %3").arg(currentCount).arg(targetCount).arg(unit);
}

QString CountHabit::serializeValue() const
{
    return QString::number(currentCount);
}

void CountHabit::deserializeValue(const QString &val)
{
    currentCount = val.toInt();
}

WorkoutHabit::WorkoutHabit(int id, int sid, QString n, Frequency f, int targetMin, int targetCnt, QString u)
    : Habit(id, sid, n, HabitType::WORKOUT, f),
      DurationHabit(id, sid, n, f, targetMin),
      CountHabit(id, sid, n, f, targetCnt, u)
{
}

QString WorkoutHabit::getProgressString() const
{
    int totalSeconds = static_cast<int>(currentMinutes * 60);
    int mins = totalSeconds / 60;
    int secs = totalSeconds % 60;
    return QString("%1:%2/%3 mins, %4/%5 %6").arg(mins).arg(secs, 2, 10, QChar('0')).arg(targetMinutes).arg(currentCount).arg(targetCount).arg(unit);
}

QString WorkoutHabit::serializeValue() const
{
    return QString("%1|%2").arg(QString::number(currentMinutes, 'f', 4)).arg(currentCount);
}

void WorkoutHabit::deserializeValue(const QString &val)
{
    QStringList parts = val.split('|');
    if (parts.size() >= 2)
    {
        currentMinutes = parts[0].toDouble();
        currentCount = parts[1].toInt();
    }
}