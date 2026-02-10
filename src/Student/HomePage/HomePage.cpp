#include "HomePage.hpp"
#include "../../Core/Input.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

HomePage::HomePage(string name, string id, int sem, string email) 
    : studentName(name), studentID(id), semester(sem), email(email) {}

void HomePage::display(unique_ptr<Routine>& routine, unique_ptr<HabitTracker>& habitTracker)
{
    clearScreen();
    showWelcomeBanner();
    showAcademicInfo();
    showTodayHighlights(routine, habitTracker);
    showUpcomingEvents(routine);
    showQuickStats(routine, habitTracker);
    
    cout << "\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}

void HomePage::showWelcomeBanner()
{
    cout << string(70, '=') << endl;
    cout << setw(50) << "Welcome to Acadence Dashboard" << endl;
    cout << string(70, '=') << endl;
}

void HomePage::showAcademicInfo()
{
    cout << "\n Academic Information\n";
    cout << string(70, '-') << endl;
    cout << "Name:      " << studentName << endl;
    cout << "Student ID: " << studentID << endl;
    cout << "Semester:  " << semester << endl;
    cout << "Email:     " << email << endl;
    cout << string(70, '-') << endl;
}

void HomePage::showTodayHighlights(unique_ptr<Routine>& routine, unique_ptr<HabitTracker>& habitTracker)
{
    cout << "\n Today's Highlights\n";
    cout << string(70, '-') << endl;
    cout << "• Check your class schedule\n";
    cout << "• Review upcoming assignments\n";
    cout << "• Track your daily habits\n";
    cout << string(70, '-') << endl;
}

void HomePage::showUpcomingEvents(unique_ptr<Routine>& routine)
{
    cout << "\n Upcoming Classes & Events\n";
    cout << string(70, '-') << endl;
    routine->showUpcomingClasses();
    cout << "View today's schedule for details\n";
    cout << string(70, '-') << endl;
}

void HomePage::showQuickStats(unique_ptr<Routine>& routine, unique_ptr<HabitTracker>& habitTracker)
{
    cout << "\n Quick Stats\n";
    cout << string(70, '-') << endl;
    cout << "• Total Classes Today: [From Routine]\n";
    cout << "• Study Hours Logged: [From Study Planner]\n";
    cout << "• Daily Habits Completed: [From Habit Tracker]\n";
    cout << string(70, '-') << endl;
}