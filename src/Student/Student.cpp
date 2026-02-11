#include "Student.hpp"
#include "Routine/Routine.hpp" // ADD: Include Routine module
#include "Habit_Tracker/Habit_Tracker.hpp"
#include "../Core/Input.hpp"
#include "../Core/Timer.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <iomanip> // ADD: For time formatting
#include <sstream>
#include <vector>

using namespace std;

// MODIFIED: Constructor now creates Routine object
Student::Student(string u, string p, string n, string e, string d, string ID, int sem)
    : User(u, p, n, e, d), Student_ID(ID), semester(sem)
{
    routine = make_unique<Routine>(username);           // ADD: Create routine
    habitTracker = make_unique<HabitTracker>(username); // ADD: Create habit tracker
    homePage = make_unique<HomePage>(n, ID, sem, e);    // ADD: Create home page
}

// MODIFIED: Menu now includes routine options (1-4)
void Student::showMenu()
{
    clearScreen();
    string name = getName();
    int choice;

    // ADD: Show welcome and class reminders
    cout << "\n--- Welcome back, " << name << " ---\n";
    routine->displayReminders();
    viewAttendence();
    do
    {
        cout << string(50, '-') << endl;
        cout << "--- Student Dashboard: " << " ---\n";
        cout << "1. Personal Home Page\n";    // NEW: Homepage
        cout << "2. View Today's Schedule\n"; // CHANGED: Now uses Routine
        cout << "3. View Weekly Schedule\n";  // ADD: New routine feature
        cout << "4. Check Class Reminders\n"; // ADD: New routine feature
        cout << "5. Edit Routine\n";          // ADD: New routine feature
        cout << "6. View Notices\n";          // CHANGED: Was option 2
        cout << "7. View Grades\n";           // CHANGED: Was option 3
        cout << "8. Study Planner\n";         // CHANGED: Was option 4
        cout << "9. Habit Tracker\n";         // CHANGED: Was option 5
        cout << "10. Change Password\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            showPersonalHomePage(); // NEW: Show personal home page
            break;
        case 2:
            routine->viewTodaySchedule(); // CHANGED: Use routine module
            break;
        case 3:
            routine->viewWeeklySchedule(); // ADD: New routine feature
            break;
        case 4:
            routine->checkClassReminders(); // ADD: New routine feature
            routine->displayReminders();
            break;
        case 5:
            routine->editRoutine(); // ADD: New routine feature
            break;
        case 6:
            viewNotices(); // UNCHANGED: Just moved
            break;
        case 7:
            viewGrades(); // UNCHANGED: Just moved
            break;
        case 8:
            checkStudyPlanner(); // UNCHANGED: Just moved
            break;
        case 9:
            habitTracker->menu(); // CHANGED: Use new HabitTracker
            break;
        case 10:
            changePassword();
            break;
        case 0:
            cout << "Logging out...\n";
            break;
        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}

void Student::showPersonalHomePage()
{
    clearScreen();
    homePage->display(routine, habitTracker);
}

// REMOVED: Old hardcoded viewSchedule() - now handled by Routine module

// ==================== ALL YOUR EXISTING CODE BELOW (UNCHANGED) ====================

void Student::changePassword()
{
    clearScreen();
    cout << "\n--- Change Password ---\n";
    string newPassword;
    inputPassword(newPassword);
    setPassword(newPassword);

    // Update users.txt to persist the new password
    ifstream inFile("users.txt");
    vector<string> lines;
    string line;
    bool found = false;

    while (getline(inFile, line))
    {
        stringstream ss(line);
        string type, uName;
        ss >> type >> uName;

        if (type == "Student" && uName == username)
        {
            stringstream newLine;
            newLine << "Student " << username << " " << password << " " << name << " " << email << " " << department << " " << Student_ID << " " << semester;
            lines.push_back(newLine.str());
            found = true;
        }
        else
        {
            lines.push_back(line);
        }
    }
    inFile.close();

    ofstream outFile("users.txt");
    for (const auto &l : lines)
        outFile << l << endl;
    outFile.close();

    if (found)
        cout << "Password changed successfully.\n";
    else
        cout << "Error updating password\n";
}

void Student::viewNotices()
{
    cout << "\n[Class Notices]\n";
    ifstream file("notices.txt");
    if (!file.is_open())
    {
        cout << "No notices available.\n";
        return;
    }
    string line;
    while (getline(file, line))
    {
        cout << ">> " << line << endl;
    }
    file.close();
}

void Student::viewGrades()
{
    cout << "\n[Your Grades]\n";
    ifstream file("grades.txt");
    if (!file.is_open())
    {
        cout << "No grades recorded yet.\n";
        return;
    }
    string sName, subject;
    double grade;
    bool found = false;
    while (file >> sName >> subject >> grade)
    {
        if (sName == username)
        {
            cout << "Subject: " << subject << " | Grade: " << grade;
            if (grade < 60)
                cout << " (Warning: Low Grade!)";
            cout << endl;
            found = true;
        }
    }
    if (!found)
        cout << "No grades found for you.\n";
    file.close();
}

void Student::checkStudyPlanner()
{
    cout << "\n[Study Planner]\n";
    cout << "1. View Tasks\n";
    cout << "2. Start Focus Timer\n";
    cout << "0. Back\n";
    cout << "Enter choice: ";
    int choice = getInt();

    if (choice == 1)
    {
        cout << "\n[X] Review C++ Pointers\n";
        cout << "[ ] Complete Calculus Assignment (Deadline: Tomorrow)\n";
        cout << "[ ] Prepare for Physics Quiz\n";
        cout << "\n(Task management features coming soon)\n";
    }
    else if (choice == 2)
    {
        int mins;
        cout << "Enter duration (minutes): ";
        cin >> mins;
        Timer timer;
        if (timer.start(mins))
        {
            cout << "Focus session recorded!\n";
        }
    }
}

void Student::viewAttendence()
{
    // cout << "\n[Attendence]\n";
    ifstream file("../attendence.txt");
    if (!file.is_open())
    {
        cout << "No Attendence recorded yet.\n";
        return;
    }

    string sName, subject;
    int total_class, taken_class, attended_class;
    float percentage;
    bool found = false;
    while (file >> sName >> subject >> total_class >> taken_class >> attended_class >> percentage)
    {
        if (sName == username)
        {
            cout << "Your Attendence for " << subject << endl;
            cout << "Attendence: " << percentage << endl;
            found = true;

            if (percentage >= 85)
            {
                cout << "Your Attendence is 85% or more" << endl;
                cout << "Eligible for examination." << endl;
            }
            else if (percentage <= 85)
            {
                cout << "Your Attendence is less than 85%" << endl;

                int remaining_class = total_class - taken_class;
                int max_attended = attended_class + remaining_class;
                int max_percentage = (max_attended * 100) / total_class;

                if (percentage >= 85)
                {
                    cout << "****Your Attendence is up to Mark****" << endl;
                    cout << "****Eligible for examination.***" << endl;
                }
                else if (max_percentage >= 85)
                {
                    cout << "**** Warning!!!! Attendance below 85%.****" << endl;
                    cout << "**** Attend all remaining classes to be eligible.****" << endl;
                }
                else
                {
                    cout << "**** Warning!!!!Attendance cannot reach 85% even if all remaining classes are attended.****" << endl;
                    cout << "**** Please meet the course teacher.****" << endl;
                }
            }
        }
    }
    if (!found)
        cout << "****No attendance record found for you.****\n";

    file.close();
}