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