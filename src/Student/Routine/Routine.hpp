#pragma once
#include <string>
#include <vector>
#include <ctime>

struct ClassSchedule
{
    std::string day;
    std::string startTime;
    std::string endTime;
    std::string courseCode;
    std::string courseName;
    std::string room;
    std::string instructor;
};

struct ClassReminder
{
    ClassSchedule classInfo;
    int minutesUntil;
    bool isToday;
    bool isUpcoming;
};

class Routine
{
private:
    std::string username;
    std::vector<ClassSchedule> weeklySchedule;
    std::vector<ClassReminder> activeReminders;

public:
    Routine(const std::string& user);
    ~Routine();
    
    void loadRoutineFromFile();
    void viewTodaySchedule();
    void viewWeeklySchedule();
    void editRoutine();
    void saveRoutineToFile();
    
    void checkClassReminders();
    void showNextClass();
    void showUpcomingClasses();
    void displayReminders();
    void checkForImmediateClasses();
    
    const std::vector<ClassSchedule>& getWeeklySchedule() const { return weeklySchedule; }
    const std::vector<ClassReminder>& getActiveReminders() const { return activeReminders; }
    
private:
    std::string getCurrentDay() const;
    int getCurrentTimeInMinutes() const;
    int parseTimeToMinutes(const std::string& timeStr) const;
    ClassReminder createReminder(const ClassSchedule& cls) const;
    bool isClassToday(const ClassSchedule& cls) const;
    bool isClassUpcoming(const ClassSchedule& cls, int currentTimeInMin) const;
    int getMinutesUntilClass(const ClassSchedule& cls, int currentTimeInMin) const;
};
