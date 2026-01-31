#pragma once
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include "../../Core/Timer.hpp"
#include "../../Core/Input.hpp"

using namespace std;

enum class HabitType
{
    DURATION,
    PRAYER,
    COUNT
};
enum class Frequency
{
    DAILY,
    WEEKLY
};

class Habit
{
protected:
    string name;
    HabitType type;
    Frequency frequency;
    int streak;
    time_t lastUpdated;
    bool isCompleted;

public:
    Habit(string n, HabitType t, Frequency f);
    virtual ~Habit() = default;

    string getName() const { return name; }
    bool getCompleted() const { return isCompleted; }

    virtual void display() const = 0;
    virtual void perform() = 0;
    virtual string serialize() const = 0;
    virtual void deserialize(stringstream &ss) = 0;

    void checkReset();
    void markComplete();
    string getFrequencyString() const;
    string getTypeString() const;
};

class DurationHabit : public Habit
{
    int targetMinutes;
    int currentMinutes;

public:
    DurationHabit(string n, Frequency f, int target);
    void display() const override;
    void perform() override;
    string serialize() const override;
    void deserialize(stringstream &ss) override;
};

class CountHabit : public Habit
{
    int targetCount;
    int currentCount;
    string unit;

public:
    CountHabit(string n, Frequency f, int target, string u);
    void display() const override;
    void perform() override;
    string serialize() const override;
    void deserialize(stringstream &ss) override;
};

class PrayerHabit : public Habit
{
    bool prayers[5]; // Fajr, Dhuhr, Asr, Maghrib, Isha
public:
    PrayerHabit();
    void display() const override;
    void perform() override;
    string serialize() const override;
    void deserialize(stringstream &ss) override;
};

class HabitTracker
{
    string username;
    vector<shared_ptr<Habit>> habits;
    string getFileName() const { return username + "_habit.txt"; }

public:
    HabitTracker(string user);
    void loadHabits();
    void saveHabits();
    void menu();

private:
    void createHabit();
    void deleteHabit();
    void viewHabits();
    void checkReminders();
};
