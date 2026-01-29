#pragma once
#include "../App/Person.hpp"
#include <vector>
#include <string>
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

class Student : public Person
{
public:
    Student(std::string u, std::string p, std::string dept, std::string prog, int sem);
    void showMenu() override;

   
    std::string getDepartment() const { return department; }
    std::string getProgram() const { return program; }
    int getSemester() const { return semester; }

private:
  
    std::string department;
    std::string program;
    int semester;

    
    std::vector<ClassSchedule> weeklySchedule;
    std::vector<ClassReminder> activeReminders;

    
    void loadRoutineFromFile();
    void viewSchedule();
    void viewTodaySchedule();
    void viewWeeklySchedule();
    void editRoutine();
    void saveRoutineToFile();
    
   
    void checkClassReminders();
    void showNextClass();
    void showUpcomingClasses();
    void displayReminders();
    void checkForImmediateClasses();
    
  
    std::string getCurrentDay() const;
    int getCurrentTimeInMinutes() const;
    int parseTimeToMinutes(const std::string& timeStr) const;
    ClassReminder createReminder(const ClassSchedule& cls) const;
    bool isClassToday(const ClassSchedule& cls) const;
    bool isClassUpcoming(const ClassSchedule& cls, int currentTimeInMin) const;
    int getMinutesUntilClass(const ClassSchedule& cls, int currentTimeInMin) const;
    
   
    void viewNotices();
    void viewGrades();
    void checkStudyPlanner();
    void checkHabitTracker();
};