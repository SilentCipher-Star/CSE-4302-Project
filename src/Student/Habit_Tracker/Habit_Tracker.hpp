#pragma once
#include <string>
#include <iostream>
using namespace std;

class Habit
{

private:
    string activity_name;
    int time;
   
    int streak;
    int frequency;
    string type; // daily,weekly,custom_days
    bool check_in;
    int calender;
    bool reminder;

protected:
    void SetName(string activity);
    void SetTime(int minutes);
    
    void SetFrequency(int times);
    void SetType(string type);
    void SetCalender(int days);

    string GetName();
    int GetTime();
    
    int GetFrequency();
    string GetType();
    int GetCalender();
    bool Check();
    bool Reminder();
    
    int GetStreak();

public:
    Habit(string name, int time, int duration, int freq, string type, int calender, bool reminder);
    ~Habit();

    // funtion
    bool completed();
};

class Prayer: public Habit{
private:
    string waqt;

public:
    void SetWaqt(string waqt);
    string GetWaqt();
};

class Workout: public Habit{

private:
    int duration;
    int progress_timer;

protected:
    void SetDuration(int minutes);
    int GetDuration();
    int GetProgress();
};

class Sports: public Habit{

private:
    int duration;
    int progress_timer;

protected:
    void SetDuration(int minutes);
    int GetDuration();
    int GetProgress();
};

class Extra_Curriculum : public Habit{

private:
    int duration;
    int progress_timer;

protected:
    void SetDuration(int minutes);
    int GetDuration();
    int GetProgress();
};
