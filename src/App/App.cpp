#include "App.hpp"
#include "../Student/Student.hpp"
#include "../Teacher/Teacher.hpp"
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
    if (!file.is_open()) return;

    string type, username, password;
    while (file >> type >> username >> password)
    {
        if (type == "Student")
            users.push_back(make_shared<Student>(username, password));
        else if (type == "Teacher")
            users.push_back(make_shared<Teacher>(username, password));
    }
    file.close();
}

void App::saveUser(const string& type, const string& username, const string& password)
{
    ofstream file("users.txt", ios::app);
    if (file.is_open())
    {
        file << type << " " << username << " " << password << endl;
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
        cout << "2. Create New User\n";
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
            if (password.length() < 6)
            {
                cout << "Password must be at least 6 characters.\n";
                continue;
            }

            bool loggedIn = false;
            for (auto &user : users)
            {
                if (user->getUsername() == username && user->authenticate(password))
                {
                    loggedIn = true;
                    user->showMenu();
                    break;
                }
            }

            if (!loggedIn)
                cout << "Invalid credentials!\n";
        }
        else if (choice == 2)
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
            if (type == 1)
            {
                users.push_back(make_shared<Student>(username, password));
                saveUser("Student", username, password);
                cout << "Student account created!\n";
            }
            else if (type == 2)
            {
                users.push_back(make_shared<Teacher>(username, password));
                saveUser("Teacher", username, password);
                cout << "Teacher account created!\n";
            }
            else
            {
                cout << "Invalid role selected.\n";
            }
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