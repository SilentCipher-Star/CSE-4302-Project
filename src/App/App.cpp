#include "App.hpp"
#include "../Student/Student.hpp"
#include "../Teacher/Teacher.hpp"
#include "../Admin/Admin.hpp"
#include "../Core/Input.hpp"
#include <iostream>
#include <fstream>

using namespace std;

App::App() : isRunning(true)
{
    loadUsers();
}

void App::loadUsers()
{
    ifstream file("users.txt");

    if (!file.is_open())
    {
        cout << "Error: Could not open users file\n"; // Error handling
        return;
    }

    admin = make_shared<Admin>("admin", "admin", "Admin");

    string type;
    while (file >> type)
    {
        if (type == "Teacher")
        {
            string u, p, n, e, d, ID, desig;
            file >> u >> p >> n >> e >> d >> ID >> desig;
            auto teacher = make_shared<Teacher>(u, p, "Teacher", n, e, d, ID, desig);
            users.push_back(teacher);
        }
        else if (type == "Student")
        {
            string u, p, n, e, d, ID;
            int sem;
            file >> u >> p >> n >> e >> d >> ID >> sem;
            auto student = make_shared<Student>(u, p, "Student", n, e, d, ID, sem);
            users.push_back(student);
        }
    }
    file.close();
}

void App::run()
{
    while (isRunning)
    {
        cout << "\n1. Login\n0. Exit\nChoice: ";
        int choice = getInt(); // Only integer inputs
        if (choice == 0)
            isRunning = false;
        else if (choice == 1)
        {
            string u, p;
            cout << "Username: ";
            cin >> u;
            cout << "Password: ";
            cin >> p;
            bool found = false;
            if (admin->getUsername() == u && admin->getPassword() == p)
            {
                found = true;
                admin->showMenu();
            }
            else
            {
                for (auto &user : users)
                {
                    if (user->getUsername() == u && user->getPassword() == p)
                    {
                        found = true;
                        user->showMenu();
                        break;
                    }
                }
            }
            if (!found)
                cout << "Invalid credentials.\n";
        }
    }
}