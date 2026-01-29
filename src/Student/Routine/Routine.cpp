#include "Routine.hpp"
#include "../../Core/Input.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace std;

Routine::Routine(const string& user) : username(user)
{
    loadRoutineFromFile();
    checkClassReminders();
}

Routine::~Routine()
{
}

void Routine::loadRoutineFromFile()
{
    string filename = "routines/" + username + "_routine.txt";
    ifstream file(filename);
    
    if (!file.is_open())
    {
        file.open("routines/default_routine.txt");
        if (!file.is_open())
        {
            cout << "[WARNING] No routine file found for " << username << ".\n";
            return;
        }
        cout << "[INFO] Loading default routine...\n";
    }

    weeklySchedule.clear();
    string line;
    int lineCount = 0;
    
    while (getline(file, line))
    {
        lineCount++;
        
        if (line.empty() || line[0] == '#') continue;
        
        stringstream ss(line);
        ClassSchedule cls;
        
        if (!(ss >> cls.day >> cls.startTime >> cls.endTime >> cls.courseCode 
              >> cls.courseName >> cls.room >> cls.instructor))
        {
            cout << "[WARNING] Invalid format on line " << lineCount << "\n";
            continue;
        }
        
        replace(cls.courseName.begin(), cls.courseName.end(), '_', ' ');
        replace(cls.courseCode.begin(), cls.courseCode.end(), '_', ' ');
        replace(cls.room.begin(), cls.room.end(), '_', ' ');
        replace(cls.instructor.begin(), cls.instructor.end(), '_', ' ');
        
        weeklySchedule.push_back(cls);
    }
    
    file.close();
    
    if (weeklySchedule.empty())
    {
        cout << "[WARNING] No classes found in routine file.\n";
    }
    else
    {
        cout << "[SUCCESS] Loaded " << weeklySchedule.size() << " classes from routine.\n";
    }
}

void Routine::viewTodaySchedule()
{
    cout << "\n=====================================================\n";
    cout << "           TODAY'S CLASS SCHEDULE                    \n";
    cout << "=====================================================\n";
    
    if (weeklySchedule.empty())
    {
        cout << "No schedule loaded.\n";
        return;
    }
    
    string today = getCurrentDay();
    int currentTime = getCurrentTimeInMinutes();
    bool foundClasses = false;
    
    cout << "\nDay: " << today << "\n";
    cout << "Current Time: " << (currentTime / 60) << ":"
         << setfill('0') << setw(2) << (currentTime % 60) << "\n\n";
    
    for (const auto& cls : weeklySchedule)
    {
        if (cls.day == today)
        {
            foundClasses = true;
            int classTime = parseTimeToMinutes(cls.startTime);
            int minutesUntil = classTime - currentTime;
            
            cout << "-----------------------------------------------------\n";
            cout << "Time: " << cls.startTime << " - " << cls.endTime;
            
            if (minutesUntil < -90)
            {
                cout << " (Completed) ";
            }
            else if (minutesUntil < 0 && minutesUntil > -90)
            {
                cout << " (IN PROGRESS!) ";
            }
            else if (minutesUntil <= 15 && minutesUntil >= 0)
            {
                cout << " *** STARTING SOON! ***";
            }
            else if (minutesUntil <= 60 && minutesUntil > 15)
            {
                cout << " (In " << minutesUntil << " mins) ";
            }
            
            cout << "\n";
            cout << "Course: " << cls.courseCode << " - " << cls.courseName << "\n";
            cout << "Room: " << cls.room << "\n";
            cout << "Instructor: " << cls.instructor << "\n";
            cout << "-----------------------------------------------------\n\n";
        }
    }
    
    if (!foundClasses)
    {
        cout << "No classes today! Enjoy your day off.\n";
    }
    
    showNextClass();
}

void Routine::viewWeeklySchedule()
{
    cout << "\n=====================================================\n";
    cout << "           WEEKLY CLASS SCHEDULE                     \n";
    cout << "=====================================================\n";
    
    if (weeklySchedule.empty())
    {
        cout << "No schedule loaded.\n";
        return;
    }
    
    string today = getCurrentDay();
    string days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    
    for (const string& day : days)
    {
        bool hasClasses = false;
        
        for (const auto& cls : weeklySchedule)
        {
            if (cls.day == day)
            {
                hasClasses = true;
                break;
            }
        }
        
        if (!hasClasses) continue;
        
        cout << "\n==============================================================\n";
        cout << "  " << day;
        if (day == today)
            cout << " (TODAY) **";
        cout << "\n==============================================================\n";
        
        for (const auto& cls : weeklySchedule)
        {
            if (cls.day == day)
            {
                cout << cls.startTime << " - " << cls.endTime << "\n";
                cout << "   " << cls.courseCode << " - " << cls.courseName << "\n";
                cout << "   " << cls.room << " | " << cls.instructor << "\n\n";
            }
        }
    }
}

void Routine::editRoutine()
{
    cout << "\n========================================\n";
    cout << "          EDIT ROUTINE                  \n";
    cout << "========================================\n";
    
    cout << "\n1. Add Class\n";
    cout << "2. Remove Class\n";
    cout << "3. View Current Routine\n";
    cout << "0. Back\n";
    cout << "Choice: ";
    
    int choice = getInt();
    cin.ignore();
    
    if (choice == 1)
    {
        ClassSchedule newClass;
        
        cout << "\nEnter Day (Monday/Tuesday/etc.): ";
        getline(cin, newClass.day);
        
        cout << "Enter Start Time (HH:MM): ";
        getline(cin, newClass.startTime);
        
        cout << "Enter End Time (HH:MM): ";
        getline(cin, newClass.endTime);
        
        cout << "Enter Course Code: ";
        getline(cin, newClass.courseCode);
        
        cout << "Enter Course Name: ";
        getline(cin, newClass.courseName);
        
        cout << "Enter Room: ";
        getline(cin, newClass.room);
        
        cout << "Enter Instructor: ";
        getline(cin, newClass.instructor);
        
        weeklySchedule.push_back(newClass);
        saveRoutineToFile();
        
        cout << "\n[SUCCESS] Class added successfully!\n";
    }
    else if (choice == 2)
    {
        if (weeklySchedule.empty())
        {
            cout << "No classes to remove.\n";
            return;
        }
        
        viewWeeklySchedule();
        
        cout << "\nEnter class number to remove (1-" << weeklySchedule.size() << "): ";
        int num = getInt();
        
        if (num >= 1 && num <= (int)weeklySchedule.size())
        {
            weeklySchedule.erase(weeklySchedule.begin() + num - 1);
            saveRoutineToFile();
            cout << "[SUCCESS] Class removed!\n";
        }
        else
        {
            cout << "[ERROR] Invalid number.\n";
        }
    }
    else if (choice == 3)
    {
        viewWeeklySchedule();
    }
}

void Routine::saveRoutineToFile()
{
    string filename = "routines/" + username + "_routine.txt";
    ofstream file(filename);
    
    if (!file.is_open())
    {
        cout << "[ERROR] Could not save routine.\n";
        return;
    }
    
    file << "# Routine for " << username << "\n";
    file << "# Format: Day StartTime EndTime CourseCode CourseName Room Instructor\n";
    file << "# Use underscores for spaces in multi-word fields\n\n";
    
    for (const auto& cls : weeklySchedule)
    {
        string courseName = cls.courseName;
        string courseCode = cls.courseCode;
        string room = cls.room;
        string instructor = cls.instructor;
        
        replace(courseName.begin(), courseName.end(), ' ', '_');
        replace(courseCode.begin(), courseCode.end(), ' ', '_');
        replace(room.begin(), room.end(), ' ', '_');
        replace(instructor.begin(), instructor.end(), ' ', '_');
        
        file << cls.day << " " << cls.startTime << " " << cls.endTime << " "
             << courseCode << " " << courseName << " " << room << " " << instructor << "\n";
    }
    
    file.close();
    cout << "[SUCCESS] Routine saved successfully.\n";
}

void Routine::checkClassReminders()
{
    activeReminders.clear();
    
    if (weeklySchedule.empty())
        return;
    
    int currentTime = getCurrentTimeInMinutes();
    
    for (const auto& cls : weeklySchedule)
    {
        if (isClassToday(cls))
        {
            int minutesUntil = getMinutesUntilClass(cls, currentTime);
            
            if (minutesUntil >= 0 && minutesUntil <= 60)
            {
                ClassReminder reminder = createReminder(cls);
                activeReminders.push_back(reminder);
            }
        }
    }
}

void Routine::showNextClass()
{
    if (weeklySchedule.empty())
        return;
    
    string today = getCurrentDay();
    int currentTime = getCurrentTimeInMinutes();
    
    ClassSchedule* nextClass = nullptr;
    int minTimeUntil = INT_MAX;
    
    for (auto& cls : weeklySchedule)
    {
        if (cls.day == today)
        {
            int classTime = parseTimeToMinutes(cls.startTime);
            int minutesUntil = classTime - currentTime;
            
            if (minutesUntil > 0 && minutesUntil < minTimeUntil)
            {
                minTimeUntil = minutesUntil;
                nextClass = &cls;
            }
        }
    }
    
    if (nextClass)
    {
        cout << "\n[NEXT CLASS] " << nextClass->courseName 
             << " at " << nextClass->startTime;
        
        if (minTimeUntil <= 15)
        {
            cout << " *** STARTING IN " << minTimeUntil << " MINUTES! ***\n";
        }
        else if (minTimeUntil <= 60)
        {
            cout << " (in " << minTimeUntil << " minutes)\n";
        }
        else
        {
            cout << "\n";
        }
    }
    else
    {
        cout << "\n[INFO] No more classes today!\n";
    }
}

void Routine::showUpcomingClasses()
{
    if (weeklySchedule.empty())
        return;
    
    string today = getCurrentDay();
    int currentTime = getCurrentTimeInMinutes();
    int count = 0;
    
    cout << "\n📋 Upcoming Classes Today:\n";
    
    for (const auto& cls : weeklySchedule)
    {
        if (cls.day == today)
        {
            int classTime = parseTimeToMinutes(cls.startTime);
            int minutesUntil = classTime - currentTime;
            
            if (minutesUntil > 0 && count < 3)
            {
                cout << "• " << cls.startTime << " - " << cls.courseName;
                if (minutesUntil <= 60)
                    cout << " (in " << minutesUntil << " mins)";
                cout << "\n";
                count++;
            }
        }
    }
    
    if (count == 0)
    {
        cout << "No upcoming classes.\n";
    }
}

void Routine::displayReminders()
{
    if (activeReminders.empty())
        return;
    
    cout << "\n=====================================================\n";
    cout << "              CLASS REMINDERS                        \n";
    cout << "=====================================================\n";
    
    for (const auto& reminder : activeReminders)
    {
        cout << "\n";
        
        if (reminder.minutesUntil <= 5)
        {
            cout << "*** URGENT: ";
        }
        else if (reminder.minutesUntil <= 15)
        {
            cout << "** WARNING: ";
        }
        else
        {
            cout << "* NOTICE: ";
        }
        
        cout << reminder.classInfo.courseName << "\n";
        cout << "   Time: " << reminder.classInfo.startTime << "\n";
        cout << "   Room: " << reminder.classInfo.room << "\n";
        cout << "   Starting in: " << reminder.minutesUntil << " minutes\n";
        
        if (reminder.minutesUntil <= 15)
        {
            cout << "   Tip: Start heading to class now!\n";
        }
    }
    
    cout << "\n";
}

void Routine::checkForImmediateClasses()
{
    checkClassReminders();
    
    for (const auto& reminder : activeReminders)
    {
        if (reminder.minutesUntil <= 15)
        {
            cout << "\n[REMINDER] " << reminder.classInfo.courseName 
                 << " starts in " << reminder.minutesUntil << " minutes!\n";
        }
    }
}

string Routine::getCurrentDay() const
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    string days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return days[ltm->tm_wday];
}

int Routine::getCurrentTimeInMinutes() const
{
    time_t now = time(0);
    tm *ltm = localtime(&now);
    return ltm->tm_hour * 60 + ltm->tm_min;
}

int Routine::parseTimeToMinutes(const string& timeStr) const
{
    int hour, minute;
    char colon;
    stringstream ss(timeStr);
    ss >> hour >> colon >> minute;
    return hour * 60 + minute;
}

ClassReminder Routine::createReminder(const ClassSchedule& cls) const
{
    ClassReminder reminder;
    reminder.classInfo = cls;
    reminder.isToday = true;
    reminder.minutesUntil = getMinutesUntilClass(cls, getCurrentTimeInMinutes());
    reminder.isUpcoming = (reminder.minutesUntil >= 0 && reminder.minutesUntil <= 60);
    return reminder;
}

bool Routine::isClassToday(const ClassSchedule& cls) const
{
    return cls.day == getCurrentDay();
}

bool Routine::isClassUpcoming(const ClassSchedule& cls, int currentTimeInMin) const
{
    if (!isClassToday(cls))
        return false;
    
    int minutesUntil = getMinutesUntilClass(cls, currentTimeInMin);
    return (minutesUntil >= 0 && minutesUntil <= 60);
}

int Routine::getMinutesUntilClass(const ClassSchedule& cls, int currentTimeInMin) const
{
    int classTime = parseTimeToMinutes(cls.startTime);
    return classTime - currentTimeInMin;
}
