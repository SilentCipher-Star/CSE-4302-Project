#include "Student.hpp"
#include "Habit_Tracker/Habit_Tracker.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

using namespace std;

Student::Student(string u, string p, string dept, string prog, int sem)
    : Person(u, p), department(dept), program(prog), semester(sem) {}

void Student::showMenu()
{
    int choice;
    do
    {
        cout << "\n--- Student Dashboard: " << username << " ---\n";
        cout << "1. View Today's Schedule\n";
        cout << "2. View Notices\n";
        cout << "3. View Grades\n";
        cout << "4. Study Planner\n";
        cout << "5. Habit Tracker\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            viewSchedule();
            break;
        case 2:
            viewNotices();
            break;
        case 3:
            viewGrades();
            break;
        case 4:
            checkStudyPlanner();
            break;
        case 5:
            checkHabitTracker();
            break;
        case 0:
            cout << "Logging out...\n";
            break;
        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}

void Student::viewSchedule()
{
    cout << "\n[Schedule]\n";
    cout << "- 09:00 AM: Object Oriented Programming\n";
    cout << "- 11:00 AM: Linear Algebra\n";
    cout << "- 02:00 PM: Physics Lab (Next in 30 mins!)\n";
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
    cout << "[X] Review C++ Pointers\n";
    cout << "[ ] Complete Calculus Assignment (Deadline: Tomorrow)\n";
    cout << "[ ] Prepare for Physics Quiz\n";

    char done;
    cout << "Did you finish the Calculus Assignment? (y/n): ";
    cin >> done;
    if (done == 'y' || done == 'Y')
        cout << "Great job! Marking as complete.\n";
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
            cout << "Time: " << time << " mins | Duration: " << duration << " mins" << endl;
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
