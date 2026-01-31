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
    routine = make_unique<Routine>(username); // ADD: Create routine
}

// MODIFIED: Menu now includes routine options (1-4)
void Student::showMenu()
{
    int choice;

    // ADD: Show welcome and class reminders
    cout << "\n--- Welcome back, " << name << " ---\n";
    routine->displayReminders();

    do
    {
        cout << string(50, '-') << endl;
        cout << "--- Student Dashboard: " << " ---\n";
        cout << "1. View Today's Schedule\n"; // CHANGED: Now uses Routine
        cout << "2. View Weekly Schedule\n";  // ADD: New routine feature
        cout << "3. Check Class Reminders\n"; // ADD: New routine feature
        cout << "4. Edit Routine\n";          // ADD: New routine feature
        cout << "5. View Notices\n";          // CHANGED: Was option 2
        cout << "6. View Grades\n";           // CHANGED: Was option 3
        cout << "7. Study Planner\n";         // CHANGED: Was option 4
        cout << "8. Habit Tracker\n";         // CHANGED: Was option 5
        cout << "9. Change Password\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            routine->viewTodaySchedule(); // CHANGED: Use routine module
            break;
        case 2:
            routine->viewWeeklySchedule(); // ADD: New routine feature
            break;
        case 3:
            routine->checkClassReminders(); // ADD: New routine feature
            routine->displayReminders();
            break;
        case 4:
            routine->editRoutine(); // ADD: New routine feature
            break;
        case 5:
            viewNotices(); // UNCHANGED: Just moved
            break;
        case 6:
            viewGrades(); // UNCHANGED: Just moved
            break;
        case 7:
            checkStudyPlanner(); // UNCHANGED: Just moved
            break;
        case 8:
            checkHabitTracker(); // UNCHANGED: Just moved
            break;
        case 9:
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

// REMOVED: Old hardcoded viewSchedule() - now handled by Routine module

// ==================== ALL YOUR EXISTING CODE BELOW (UNCHANGED) ====================

void Student::changePassword()
{
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

void Student::checkHabitTracker()
{
    cout << "\n[Habit Tracker]\n";
    cout << "1. Create New Habit\n";
    cout << "2. View Habits\n";
    cout << "3. Check-in (Mark Done)\n";
    cout << "4. Delete Habit\n";
    cout << "0. Exit\n";
    cout << "Enter choice: ";
    int choice;
    cin >> choice;
    switch (choice)
    {
    case 1:
        createHabit();
        break;
    case 2:
        viewHabit();
        break;
    case 3:
        markHabitDone();
        break;
    case 4:
        deleteHabit();
        break;
    case 0:
        cout << "Exiting Habit Tracker.\n";
        break;
    default:
        cout << "Invalid choice.\n";
    }
}

void Student::createHabit()
{
    cout << "\n[Create New Habit]\n";
    string name, type;
    int time, duration, frequency, calender;
    bool reminder;

    cout << "Enter Habit Name: ";
    cin >> name;
    cout << "Enter Habit Time (minutes): ";
    cin >> time;
    cout << "Enter Habit Duration (minutes): ";
    cin >> duration;
    cout << "Enter Habit Frequency: ";
    cin >> frequency;
    cout << "Enter Habit Type: ";
    cin >> type;
    cout << "Enter Habit Calender (days): ";
    cin >> calender;
    cout << "Enter Habit Reminder (1 for Yes, 0 for No): ";
    cin >> reminder;

    ofstream file("habits.txt", ios::app);
    if (file.is_open())
    {
        // Format: username name time duration frequency type calendar reminder streak check_in
        file << username << " " << name << " " << time << " " << duration << " " << frequency << " " << type << " " << calender << " " << reminder << " " << 0 << " " << 0 << endl;
        cout << "Habit created successfully.\n";
        file.close();
    }
    else
    {
        cout << "Error opening habits file.\n";
    }
}

void Student::viewHabit()
{
    cout << "\n[View Habit]\n";
    ifstream file("habits.txt");
    if (!file.is_open())
    {
        cout << "No habits available.\n";
        return;
    }

    string uName, name, type;
    int time, duration, frequency, calender, streak;
    bool reminder, check_in;
    bool found = false;

    while (file >> uName >> name >> time >> duration >> frequency >> type >> calender >> reminder >> streak >> check_in)
    {
        if (uName == username)
        {
            cout << "--------------------------------\n";
            cout << "Habit Name: " << name << endl;
            cout << "Time: " << time << " mins | Duration: " << duration << " " << endl;
            cout << "Frequency: " << frequency << " | Type: " << type << endl;
            cout << "Calendar: " << calender << " days | Reminder: " << (reminder ? "Yes" : "No") << endl;
            cout << "Streak: " << streak << " | Checked In: " << (check_in ? "Yes" : "No") << endl;
            found = true;
        }
    }
    if (!found)
        cout << "No habits found for " << username << ".\n";
    file.close();
}

void Student::deleteHabit()
{
    cout << "\n[Delete Habit]\n";
    string targetName;
    cout << "Enter Habit Name to delete: ";
    cin >> targetName;

    ifstream file("habits.txt");
    if (!file.is_open())
    {
        cout << "Error opening habits file.\n";
        return;
    }
    ofstream tempFile("temp_habits.txt");
    if (!tempFile.is_open())
    {
        cout << "Error creating temporary file.\n";
        file.close();
        return;
    }

    string uName, name, type;
    int time, duration, frequency, calender, streak;
    bool reminder, check_in;
    bool deleted = false;

    while (file >> uName >> name >> time >> duration >> frequency >> type >> calender >> reminder >> streak >> check_in)
    {
        if (uName == username && name == targetName)
        {
            deleted = true;
            cout << "Habit '" << name << "' deleted.\n";
            continue; // Skip writing this line to temp file
        }
        tempFile << uName << " " << name << " " << time << " " << duration << " " << frequency << " " << type << " " << calender << " " << reminder << " " << streak << " " << check_in << endl;
    }
    file.close();
    tempFile.close();

    remove("habits.txt");
    rename("temp_habits.txt", "habits.txt");

    if (!deleted)
        cout << "Habit not found.\n";
}

void Student::markHabitDone()
{
    cout << "\n[Check-in Habit]\n";
    string targetName;
    cout << "Enter Habit Name to check-in: ";
    cin >> targetName;

    ifstream file("habits.txt");
    if (!file.is_open())
    {
        cout << "Error opening habits file.\n";
        return;
    }
    ofstream tempFile("temp_habits.txt");
    if (!tempFile.is_open())
    {
        cout << "Error creating temporary file.\n";
        file.close();
        return;
    }

    string uName, name, type;
    int time, duration, frequency, calender, streak;
    bool reminder, check_in;
    bool found = false;

    while (file >> uName >> name >> time >> duration >> frequency >> type >> calender >> reminder >> streak >> check_in)
    {
        if (uName == username && name == targetName)
        {
            streak++;
            check_in = true;
            found = true;
            cout << "Great job! Streak updated to " << streak << ".\n";
        }
        tempFile << uName << " " << name << " " << time << " " << duration << " " << frequency << " " << type << " " << calender << " " << reminder << " " << streak << " " << check_in << endl;
    }
    file.close();
    tempFile.close();

    remove("habits.txt");
    rename("temp_habits.txt", "habits.txt");

    if (!found)
        cout << "Habit not found.\n";
}