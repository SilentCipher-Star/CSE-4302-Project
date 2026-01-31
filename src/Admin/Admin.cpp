#include "../App/App.hpp"
#include "../App/User.hpp"
#include "../Student/Student.hpp"
#include "../Teacher/Teacher.hpp"
#include "../Admin/Admin.hpp"
#include "../Core/Input.hpp"
#include <iostream>
#include <fstream>

using namespace std;

#define admin_password "admin"

Admin::Admin(vector<shared_ptr<User>> &users) : Person("admin", admin_password, "Admin"), users(users) {}

// check if username already exists
bool Admin::check_existing_user(string username, vector<shared_ptr<User>> &users)
{
    for (auto &user : users)
    {
        if (user->getUsername() == username)
        {
            return true;
        }
        // Bug fix: Do not return false here; wait until the loop finishes
    }
    return false;
}

void Admin::showMenu()
{
    std::cout << "Logged in as Admin." << std::endl;
    int choice;
    do
    {
        cout << "\n--- Admin Menu ---\n";
        cout << "1. Add Student\n";
        cout << "2. Add Teacher\n";

        cout << "3. Delete User\n";
        cout << "4. Show User List\n";
        cout << "0. Logout\n";
        cout << "Enter choice: ";
        choice = getInt();

        if (choice == 1)
        {
            string u, p;
            // Optimal: Use the generic input helper with a lambda for specific logic
            getValidInput(u, "Username: ", [&](const string &input) -> bool
                          {
                if (!isValidInput(input))
                {
                    cout << "Invalid username. Allowed characters: a-z, A-Z, 0-9, . _ - @ # $ !\n";
                    return false;
                }
                // Note: Ensure 'users' is accessible here (e.g., passed to showMenu or stored in Admin)
                if (check_existing_user(input, users)) 
                {
                    cout << "Username already exists. Please try another.\n";
                    return false;
                }
                return true; });

            inputPassword(p);

            string name, email, dept, id;
            int sem;
            cout << "Name: ";
            cin >> name;
            cout << "Email: ";
            cin >> email;
            cout << "Department: ";
            cin >> dept;
            cout << "Student ID: ";
            cin >> id;
            cout << "Semester: ";
            sem = getInt();
            auto s = make_shared<Student>(u, p, name, email, dept, id, sem);
            users.push_back(s);
            saveUser(s);
            cout << "Student added.\n";
        }
        else if (choice == 2)
        {
            string u, p;
            getValidInput(u, "Username: ", [&](const string &input) -> bool
                          {
                if (!isValidInput(input))
                {
                    cout << "Invalid username. Allowed characters: a-z, A-Z, 0-9, . _ - @ # $ !\n";
                    return false;
                }
                if (check_existing_user(input, users))
                {
                    cout << "Username already exists. Please try another.\n";
                    return false;
                }
                return true; });

            inputPassword(p);

            string name, email, dept, id, desig;
            cout << "Name: ";
            cin >> name;
            cout << "Email: ";
            cin >> email;
            cout << "Department: ";
            cin >> dept;
            cout << "Teacher ID: ";
            cin >> id;
            cout << "Designation: ";
            cin >> desig;

            auto t = make_shared<Teacher>(u, p, name, email, dept, id, desig);
            users.push_back(t);
            saveUser(t);
            cout << "Teacher added.\n";
        }
        else if (choice == 3)
        {
            string u;
            cout << "Enter username to delete: ";
            cin >> u;
            for (auto it = users.begin(); it != users.end(); ++it)
            {
                if ((*it)->getUsername() == u)
                {
                    users.erase(it);
                    cout << "User deleted.\n";
                    ofstream file("users.txt");
                    if (!file.is_open())
                    {
                        cout << "Error opening users file.\n";
                        return;
                    }
                    for (auto &user : users)
                    {
                        // find the user and delete that line
                    }
                    file.close();
                    break;
                }
            }
        }
        else if (choice == 4)
        {
            cout << "\n--- Student List ---\n";
            for (auto &user : users)
            {
                if (auto s = dynamic_pointer_cast<Student>(user))
                {
                    cout << "Name: " << s->getName() << " | Email: " << s->getEmail() << " | Department: " << s->getDepartment() << " | ID: " << s->ID() << " | Semester: " << s->getSemester() << endl;
                }
            }
            cout << "\n--- Teacher List ---\n";
            for (auto &user : users)
            {
                if (auto t = dynamic_pointer_cast<Teacher>(user))
                {
                    cout << "Name: " << t->getName() << " | Email: " << t->getEmail() << " | Department: " << t->getDepartment() << " | ID: " << t->ID() << " | Designation: " << t->getDesignation() << endl;
                }
            }
        }
        else if (choice == 0)
        {
            cout << "Logging out...\n";
        }
        else
        {
            cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}

void Admin::saveUser(shared_ptr<User> user)
{
    ofstream file("users.txt", ios::app);
    if (!file.is_open())
        return;

    if (auto s = dynamic_pointer_cast<Student>(user))
    {
        file << "Student " << s->getUsername() << " " << s->getPassword() << " " << s->getName() << " " << s->getEmail() << " " << s->getDepartment() << " " << s->ID() << " " << s->getSemester() << endl;
    }
    else if (auto t = dynamic_pointer_cast<Teacher>(user))
    {
        file << "Teacher " << t->getUsername() << " " << t->getPassword() << " " << t->getName() << " " << t->getEmail() << " " << t->getDepartment() << " " << t->ID() << " " << t->getDesignation() << endl;
    }
    file.close();
}