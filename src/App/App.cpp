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
    users.clear();
    ifstream file("users.txt");
    if (!file.is_open())
        return;

    string type, username, password;
    while (file >> type >> username >> password)
    {
        if (type == "Student")
        {
            string dept, prog;
            int sem;
            file >> dept >> prog >> sem;
            users.push_back(make_shared<Student>(username, password, dept, prog, sem));
        }
        else if (type == "Teacher")
        {
            string desig, dept;
            file >> desig >> dept;
            users.push_back(make_shared<Teacher>(username, password, desig, dept));
        }
        else if (type == "Admin")
            users.push_back(make_shared<Admin>(username, password));
    }
    file.close();

    // Ensure at least one admin exists
    bool hasAdmin = false;
    for (const auto &user : users)
    {
        if (dynamic_pointer_cast<Admin>(user))
            hasAdmin = true;
    }
    if (!hasAdmin)
    {
        auto admin = make_shared<Admin>("admin", "admin");
        users.push_back(admin);
        saveUser(admin);
    }
}

void App::saveUser(const shared_ptr<Person> &user)
{
    ofstream file("users.txt", ios::app);
    if (file.is_open())
    {
        if (auto s = dynamic_pointer_cast<Student>(user))
        {
            file << "Student " << s->getUsername() << " " << s->getPassword() << " "
                 << s->getDepartment() << " " << s->getProgram() << " " << s->getSemester() << endl;
        }
        else if (auto t = dynamic_pointer_cast<Teacher>(user))
        {
            file << "Teacher " << t->getUsername() << " " << t->getPassword() << " "
                 << t->getDesignation() << " " << t->getDepartment() << endl;
        }
        else if (dynamic_pointer_cast<Admin>(user))
        {
            file << "Admin " << user->getUsername() << " " << user->getPassword() << endl;
        }
        file.close();
    }
}

void App::saveAllUsers()
{
    ofstream file("users.txt", ios::trunc);
    if (file.is_open())
    {
        for (const auto &user : users)
        {
            // Re-use logic similar to saveUser but for all
            if (auto s = dynamic_pointer_cast<Student>(user))
            {
                file << "Student " << s->getUsername() << " " << s->getPassword() << " "
                     << s->getDepartment() << " " << s->getProgram() << " " << s->getSemester() << endl;
            }
            else if (auto t = dynamic_pointer_cast<Teacher>(user))
            {
                file << "Teacher " << t->getUsername() << " " << t->getPassword() << " "
                     << t->getDesignation() << " " << t->getDepartment() << endl;
            }
            else if (dynamic_pointer_cast<Admin>(user))
            {
                file << "Admin " << user->getUsername() << " " << user->getPassword() << endl;
            }
        }
        file.close();
    }
}

void App::run()
{
    cout << "Welcome to Personal Academic Assistant\n";

    while (isRunning)
    {
        int choice;
        cout << "\n1. Sign In\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";
        choice = getInt();
        if (choice == 1)
        {
            string username, password;
            cout << "\n--- Login ---\n";
            cout << "Username: ";
            cin.ignore();
            getline(cin, username);
            if (!isValidCredential(username))
            {
                cout << "Invalid username characters!\n";
                continue;
            }
            cout << "Password: ";
            getline(cin, password);
            if (!isValidCredential(password))
            {
                cout << "Invalid password characters!\n";
                continue;
            }
            if (password.length() < 4)
            {
                cout << "Password must be at least 4 characters.\n";
                continue;
            }

            bool loggedIn = false;
            for (auto &user : users)
            {
                if (user->getUsername() == username && user->authenticate(password))
                {
                    loggedIn = true;
                    if (dynamic_pointer_cast<Admin>(user))
                    {
                        handleAdmin();
                    }
                    else
                    {
                        user->showMenu();
                    }
                    break;
                }
            }

            if (!loggedIn)
                cout << "Invalid credentials!\n";
        }
        else if (choice == 0)
        {
            isRunning = false;
        }
        else
        {
            cout << "Invalid choice.\n";
        }
    }
}

void App::handleAdmin()
{
    bool back = false;
    while (!back)
    {
        cout << "\n--- Admin Menu ---\n";
        cout << "1. Create New User\n";
        cout << "2. Delete User\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";
        int choice = getInt();

        if (choice == 1)
        {
            int type;
            string username, password;
            cout << "\n--- Create Account ---\n";
            cout << "1. Student\n2. Teacher\nSelect Role: ";
            type = getInt();
            cin.ignore();
            cout << "Enter Username: ";
            getline(cin, username);
            if (!isValidCredential(username))
            {
                cout << "Invalid username characters!\n";
                continue;
            }
            cout << "Enter Password: ";
            getline(cin, password);
            if (!isValidCredential(password))
            {
                cout << "Invalid password characters!\n";
                continue;
            }
            if (password.length() < 4)
            {
                cout << "Password must be at least 4 characters.\n";
                continue;
            }

            if (type == 1)
            {
                string dept, prog;
                int sem;
                cout << "Enter Department: ";
                cin >> dept;
                cout << "Enter Program: ";
                cin >> prog;
                cout << "Enter Semester: ";
                sem = getInt();

                auto newStudent = make_shared<Student>(username, password, dept, prog, sem);
                users.push_back(newStudent);
                saveUser(newStudent);
                cout << "Student account created!\n";
            }
            else if (type == 2)
            {
                string desig, dept;
                cout << "Enter Designation: ";
                cin >> desig;
                cout << "Enter Department: ";
                cin >> dept;

                auto newTeacher = make_shared<Teacher>(username, password, desig, dept);
                users.push_back(newTeacher);
                saveUser(newTeacher);
                cout << "Teacher account created!\n";
            }
            else
            {
                cout << "Invalid role selected.\n";
            }
        }
        else if (choice == 2)
        {
            string username;
            cout << "\n--- Delete User ---\n";
            cout << "Enter Username to delete: ";
            cin.ignore();
            getline(cin, username);

            bool found = false;
            for (auto it = users.begin(); it != users.end(); ++it)
            {
                if ((*it)->getUsername() == username)
                {
                    users.erase(it);
                    found = true;
                    saveAllUsers();
                    cout << "User deleted successfully.\n";
                    break;
                }
            }
            if (!found)
            {
                cout << "User not found.\n";
            }
        }
        else if (choice == 0)
        {
            back = true;
        }
    }
}