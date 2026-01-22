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
            cout << "Username: "; cin >> u;
            cout << "Password: "; cin >> p;
            cout << "Department: "; cin >> dept;
            cout << "Program: "; cin >> prog;
            cout << "Semester: "; sem = getInt();
            auto s = make_shared<Student>(u, p, dept, prog, sem);
            users.push_back(s);
            saveUser(s);
            cout << "Student added.\n";
        }
        else if (choice == 2)
        {
            string u, p, desig, dept;
            cout << "Username: "; cin >> u;
            cout << "Password: "; cin >> p;
            cout << "Designation: "; cin >> desig;
            cout << "Department: "; cin >> dept;
            auto t = make_shared<Teacher>(u, p, desig, dept);
            users.push_back(t);
            saveUser(t);
            cout << "Teacher added.\n";
        }
    } while (choice != 0);
}

void App::loadUsers()
{
    users.clear();

    // Load Admins
    ifstream file("users.txt");
    if (file.is_open())
    {
        string type, username, password;
        while (file >> type >> username >> password)
        {
            if (type == "Admin")
                users.push_back(make_shared<Admin>(username, password));
        }
        file.close();
    }

    // Load Students
    ifstream sFile("student_user.txt");
    if (sFile.is_open())
    {
        string type, username, password, dept, prog;
        int sem;
        while (sFile >> type >> username >> password >> dept >> prog >> sem)
        {
            if (type == "Student")
                users.push_back(make_shared<Student>(username, password, dept, prog, sem));
        }
        sFile.close();
    }

    // Load Teachers
    ifstream tFile("teacher_users.txt");
    if (tFile.is_open())
    {
        string type, username, password, desig, dept;
        while (tFile >> type >> username >> password >> desig >> dept)
        {
            if (type == "Teacher")
                users.push_back(make_shared<Teacher>(username, password, desig, dept));
        }
        tFile.close();
    }

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
    if (auto s = dynamic_pointer_cast<Student>(user))
    {
        ofstream file("student_user.txt", ios::app);
        if (file.is_open())
        {
            file << "Student " << s->getUsername() << " " << s->getPassword() << " "
                 << s->getDepartment() << " " << s->getProgram() << " " << s->getSemester() << endl;
            file.close();
        }
    }
    else if (auto t = dynamic_pointer_cast<Teacher>(user))
    {
        ofstream file("teacher_users.txt", ios::app);
        if (file.is_open())
        {
            file << "Teacher " << t->getUsername() << " " << t->getPassword() << " "
                 << t->getDesignation() << " " << t->getDepartment() << endl;
            file.close();
        }
    }
    else if (dynamic_pointer_cast<Admin>(user))
    {
        ofstream file("users.txt", ios::app);
        if (file.is_open())
        {
            file << "Admin " << user->getUsername() << " " << user->getPassword() << endl;
            file.close();
        }
    }
}

void App::saveAllUsers()
{
    ofstream sFile("student_user.txt", ios::trunc);
    ofstream tFile("teacher_users.txt", ios::trunc);
    ofstream aFile("users.txt", ios::trunc);

    for (const auto &user : users)
    {
        if (auto s = dynamic_pointer_cast<Student>(user))
        {
            if (sFile.is_open())
                sFile << "Student " << s->getUsername() << " " << s->getPassword() << " "
                      << s->getDepartment() << " " << s->getProgram() << " " << s->getSemester() << endl;
        }
        else if (auto t = dynamic_pointer_cast<Teacher>(user))
        {
            if (tFile.is_open())
                tFile << "Teacher " << t->getUsername() << " " << t->getPassword() << " "
                      << t->getDesignation() << " " << t->getDepartment() << endl;
        }
        else if (dynamic_pointer_cast<Admin>(user))
        {
            if (aFile.is_open())
                aFile << "Admin " << user->getUsername() << " " << user->getPassword() << endl;
        }
    }
}