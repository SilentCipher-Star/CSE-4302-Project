#include "Teacher.hpp"
#include <iostream>
#include <fstream>

using namespace std;

Teacher::Teacher(string u, string p, string n, string e, string d, string ID, string desig)
    : User(u, p, n, e, d), Teacher_ID(ID), designation(desig) {}

void Teacher::showMenu()
{
    int choice;
    do
    {
        cout << "\n--- Teacher Dashboard: " << username << " ---\n";
        cout << "1. Post Class Notice\n";
        cout << "2. Assign Grade\n";
        cout << "3. Attendence\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice)
        {
        case 1:
            postNotice();
            break;
        case 2:
            assignGrade();
            break;
        case 3:
            Attendence();
            break;
        case 0:
            cout << "Logging out...\n";
            break;
        default:
            cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}

void Teacher::postNotice()
{
    string notice;
    cout << "Enter notice message (use_underscores_for_spaces): ";
    cin >> notice; // Simple input for MVP

    ofstream file("../notices.txt", ios::app);
    if (file.is_open())
    {
        file << "[" << username << "]: " << notice << endl;
        cout << "Notice published successfully.\n";
        file.close();
    }
    else
    {
        cout << "Error opening notices file.\n";
    }
}

void Teacher::assignGrade()
{
    string sName, subject;
    double grade;
    cout << "Enter Student Username: ";
    cin >> sName;
    cout << "Enter Subject: ";
    cin >> subject;
    cout << "Enter Grade: ";
    cin >> grade;

    ofstream file("../grades.txt", ios::app);
    if (file.is_open())
    {
        file << sName << " " << subject << " " << grade << endl;
        cout << "Grade assigned successfully.\n";
        file.close();
    }
    else
    {
        cout << "Error opening grades file.\n";
    }
}

void Teacher::Attendence()
{
    int total_class, taken_class, attended_class;
    string sub, studentUserName;

    cout << "Student UserName: ";
    cin >> studentUserName;

    cout << "Subject Name: ";
    cin >> sub;

    cout << "Total Classes to be Taken: ";
    cin >> total_class;

    cout << "Total Classes Taken: ";
    cin >> taken_class;

    cout << "Total Classes Attended By the Student: ";
    cin >> attended_class;
    float percentage  = (attended_class * 100)/taken_class;
    
    ofstream file("../attendence.txt", ios::app);
    if (file.is_open())
    {
        file << studentUserName << " " << sub << " "
             << total_class << " "
             << taken_class << " "
             << attended_class << " "
             << percentage << endl;
        cout << "Attendence recorded successfully.\n";
        file.close();
    }
    else
    {
        cout << "Error opening Attendence file.\n";
    }

    
}