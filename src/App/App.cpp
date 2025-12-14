#include "App.hpp"
#include "../Student/Student.hpp"
#include "../Teacher/Teacher.hpp"
#include <iostream>

using namespace std;

App::App() : isRunning(true)
{
    loadUsers();
}

void App::loadUsers()
{
    // Simulated user database
    users.push_back(make_shared<Student>("Airah", "123"));
    users.push_back(make_shared<Student>("Abeed", "123"));
    users.push_back(make_shared<Teacher>("Rafid", "admin"));
}

void App::run()
{
    cout << "Welcome to Personal Academic Assistant\n";

    while (isRunning)
    {
        string username, password;
        cout << "\n--- Login ---\n";
        cout << "Username: ";
        cin >> username;
        cout << "Password: ";
        cin >> password;

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
        {
            cout << "Invalid credentials!\n";
        }

        char exitChoice;
        cout << "Exit application? (y/n): ";
        cin >> exitChoice;
        if (exitChoice == 'y' || exitChoice == 'Y')
            isRunning = false;
        else
            cout << "\033[2J\033[1;1H"; // Clear console
    }
}