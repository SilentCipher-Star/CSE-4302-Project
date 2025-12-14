#include "Student.hpp"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;

Student::Student(string u, string p) : Person(u, p) {}

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