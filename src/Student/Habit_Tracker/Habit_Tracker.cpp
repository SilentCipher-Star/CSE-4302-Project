#include "Habit_Tracker.hpp"

Habit::Habit(string name, int time, int duration, int freq, string type, int calender, bool reminder)
    : activity_name(name), time(time),frequency(freq), type(type), calender(calender), reminder(reminder)
{
    streak = 0;
    check_in = false;
    reminder = false;
}

Habit::~Habit()
{
    cout << "BOOOO!!!!!!!! You Loser " << endl;
}

// getter
string Habit::GetName()
{
    return activity_name;
}

int Habit::GetTime()
{
    return time;
}

int Workout::GetDuration()
{
    return duration;
}

int Sports::GetDuration()
{
    return duration;
}
int Extra_Curriculum::GetDuration()
{
    return duration;
}
int Habit::GetFrequency()
{
    return frequency;
}

string Habit::GetType()
{
    return type;
}

int Habit::GetCalender()
{
    return calender;
}

bool Habit::Reminder()
{
    return reminder;
}

bool Habit::Check()
{
    return check_in;
}

int Workout::GetProgress()
{
    return progress_timer;
}

int Sports::GetProgress()
{
    return progress_timer;
}

int Extra_Curriculum::GetProgress()
{
    return progress_timer;
}

int Habit::GetStreak()
{
    return streak;
}

// setter
void Habit::SetName(string activity)
{
    activity_name = activity;
}

void Habit::SetTime(int minutes)
{
    time = minutes;
}

void Workout::SetDuration(int minutes)
{
    duration = minutes;
}

void Habit::SetFrequency(int times)
{
    frequency = times;
}

void Habit::SetType(string type)
{
    this->type = type;
}

void Habit::SetCalender(int days)
{
    calender = days;
}

// funtion
bool Habit::completed()
{
    if (check_in == true)
    {
        return true;
    }
    else
    {
        return false;
    }
}


