#ifndef HABIT_H
#define HABIT_H

#include <QString>
#include <QDate>
#include <QStringList>

enum class HabitType
{
    DURATION,
    COUNT
};

enum class Frequency
{
    DAILY,
    WEEKLY
};

class Habit
{
public:
    int id;
    int studentId;
    QString name;
    HabitType type;
    Frequency frequency;
    int streak;
    QDate lastUpdated;
    bool isCompleted;

    Habit(int id, int sid, QString n, HabitType t, Frequency f);
    virtual ~Habit() = default;

    virtual QString getProgressString() const = 0;
    virtual QString serializeValue() const = 0;
    virtual void deserializeValue(const QString &val) = 0;
    bool checkReset();
    void markComplete();
    QString getFrequencyString() const;
    QString getTypeString() const;
};

class DurationHabit : public Habit
{
public:
    int targetMinutes;
    int currentMinutes;

    DurationHabit(int id, int sid, QString n, Frequency f, int target);
    QString getProgressString() const override;
    QString serializeValue() const override;
    void deserializeValue(const QString &val) override;
};

class CountHabit : public Habit
{
public:
    int targetCount;
    int currentCount;
    QString unit;

    CountHabit(int id, int sid, QString n, Frequency f, int target, QString u);
    QString getProgressString() const override;
    QString serializeValue() const override;
    void deserializeValue(const QString &val) override;
};

class DailyPrayerStatus
{
private:
    bool fajr;
    bool dhuhr;
    bool asr;
    bool maghrib;
    bool isha;

public:
    DailyPrayerStatus(bool f, bool d, bool a, bool m, bool i)
        : fajr(f), dhuhr(d), asr(a), maghrib(m), isha(i) {}

    bool getFajr() const { return fajr; }
    bool getDhuhr() const { return dhuhr; }
    bool getAsr() const { return asr; }
    bool getMaghrib() const { return maghrib; }
    bool getIsha() const { return isha; }
};

#endif // HABIT_H