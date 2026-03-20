#pragma once
#include <QString>
#include <QDate>
#include <QStringList>

enum class HabitType
{
    DURATION,
    COUNT,
    WORKOUT
};

enum class Frequency
{
    DAILY,
    WEEKLY
};

class Habit
{
protected:
    int id;
    int studentId;
    QString name;
    HabitType type;
    Frequency frequency;
    int streak;
    QDate lastUpdated;
    bool isCompleted;

public:
    Habit(int id, int sid, QString n, HabitType t, Frequency f);
    virtual ~Habit() = default;

    int getId() const { return id; }
    void setId(int newId) { id = newId; }
    int getStudentId() const { return studentId; }
    QString getName() const { return name; }
    HabitType getType() const { return type; }
    Frequency getFrequency() const { return frequency; }

    int getStreak() const { return streak; }
    void setStreak(int s) { streak = s; }
    QDate getLastUpdated() const { return lastUpdated; }
    void setLastUpdated(const QDate &d) { lastUpdated = d; }
    bool getIsCompleted() const { return isCompleted; }
    void setIsCompleted(bool c) { isCompleted = c; }

    virtual QString getProgressString() const = 0;
    virtual QString serializeValue() const = 0;
    virtual void deserializeValue(const QString &val) = 0;
    bool checkReset();
    void markComplete();
    QString getFrequencyString() const;
    QString getTypeString() const;
};

class DurationHabit : public virtual Habit
{
protected:
    int targetMinutes;
    double currentMinutes;

public:
    DurationHabit(int id, int sid, QString n, Frequency f, int target);

    int getTargetMinutes() const { return targetMinutes; }
    double getCurrentMinutes() const { return currentMinutes; }
    void setCurrentMinutes(double mins) { currentMinutes = mins; }
    void addMinutes(double mins) { currentMinutes += mins; }

    QString getProgressString() const override;
    QString serializeValue() const override;
    void deserializeValue(const QString &val) override;
};

class CountHabit : public virtual Habit
{
protected:
    int targetCount;
    int currentCount;
    QString unit;

public:
    CountHabit(int id, int sid, QString n, Frequency f, int target, QString u);

    int getTargetCount() const { return targetCount; }
    int getCurrentCount() const { return currentCount; }
    void setCurrentCount(int count) { currentCount = count; }
    void addCount(int count) { currentCount += count; }
    QString getUnit() const { return unit; }

    QString getProgressString() const override;
    QString serializeValue() const override;
    void deserializeValue(const QString &val) override;
};

class WorkoutHabit : public DurationHabit, public CountHabit
{
public:
    WorkoutHabit(int id, int sid, QString n, Frequency f, int targetMin, int targetCnt, QString u);
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
