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
        cout << "Error: Could not open users.txt\n";
        return;
    }

    string type;
    while (file >> type)
    {
        if (type == "Admin")
        {
            string u, p;
            file >> u >> p;
            auto admin = make_shared<Admin>(u, p);
            users.push_back(admin);
        }
        else if (type == "Teacher")
        {
            string u, p, desig, dept;
            file >> u >> p >> desig >> dept;
            auto teacher = make_shared<Teacher>(u, p, desig, dept);
            users.push_back(teacher);
        }
        else if (type == "Student")
        {
            string u, p, dept, prog;
            int sem;
            file >> u >> p >> dept >> prog >> sem;
            auto student = make_shared<Student>(u, p, dept, prog, sem);
            users.push_back(student);
        }
    }
    file.close();
}

void App::saveUser(const shared_ptr<Person> &user)
{
}

void App::saveAllUsers()
{
}




void App::run()
{
    while (isRunning)
    {
        cout << "\n1. Login\n0. Exit\nChoice: ";
        int choice = getInt();
        if (choice == 0) isRunning = false;
        else if (choice == 1)
        {
            string u, p;
            cout << "Username: "; cin >> u;
            cout << "Password: "; cin >> p;
            bool found = false;
            for (auto &user : users)
            {
                if (user->getUsername() == u && user->getPassword() == p)
                {
                    found = true;
                    if (dynamic_pointer_cast<Admin>(user)) handleAdmin();
                    else user->showMenu();
                    break;
                }
            }
            if (!found) cout << "Invalid credentials.\n";
        }
    }
}