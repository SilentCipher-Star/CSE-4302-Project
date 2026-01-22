#include "../App/App.hpp"
#include "../Student/Student.hpp"
#include "../Teacher/Teacher.hpp"
#include "../Admin/Admin.hpp"
#include "../Core/Input.hpp"
#include <iostream>
#include <fstream>

using namespace std;

void App::handleAdmin()
{
    int choice;
    do
    {
        cout << "\n--- Admin Menu ---\n";
        cout << "1. Add Student\n";
        cout << "2. Add Teacher\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";
        choice = getInt();

        if (choice == 1)
        {
            string u, p, dept, prog;
            int sem;
            cout << "Username: ";
            cin >> u;
            cout << "Password: ";
            cin >> p;
            cout << "Department: ";
            cin >> dept;
            cout << "Program: ";
            cin >> prog;
            cout << "Semester: ";
            sem = getInt();
            auto s = make_shared<Student>(u, p, dept, prog, sem);
            users.push_back(s);
            saveUser(s);
            cout << "Student added.\n";
        }
        else if (choice == 2)
        {
            string u, p, desig, dept;
            cout << "Username: ";
            cin >> u;
            cout << "Password: ";
            cin >> p;
            cout << "Designation: ";
            cin >> desig;
            cout << "Department: ";
            cin >> dept;
            auto t = make_shared<Teacher>(u, p, desig, dept);
            users.push_back(t);
            saveUser(t);
            cout << "Teacher added.\n";
        }
    } while (choice != 0);
}