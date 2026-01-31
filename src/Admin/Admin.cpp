#include "../App/App.hpp"
#include "../App/User.hpp"
#include "../Student/Student.hpp"
#include "../Teacher/Teacher.hpp"
#include "../Admin/Admin.hpp"
#include "../Core/Input.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

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
        cout << "5. Edit User\n";

        cout << "0. Logout\n";
        cout << "Enter choice: ";
        choice = getInt();

        if (choice == 1)
        {
            string u, p;
            // Optimal: Use the generic input helper with a lambda for specific logic
            getValidInput(u, "Username: ", [&](const string &input) -> bool
                          {
                if (input.length() < 3 || input.length() > 12)
                {
                    cout << "Username must be between 3 and 12 characters.\n";
                    return false;
                }
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
                if (input.length() < 3 || input.length() > 12)
                {
                    cout << "Username must be between 3 and 12 characters.\n";
                    return false;
                }
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
            getValidInput(dept, "Department (CSE, EEE, MPE, CEE, TVE): ", [](const string &input) -> bool
                          {
                if (input == "CSE" || input == "EEE" || input == "MPE" || input == "CEE" || input == "TVE") return true;
                cout << "Invalid Department. Allowed: CSE, EEE, MPE, CEE, TVE\n";
                return false; });
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
            bool found = false;
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
                        if (auto s = dynamic_pointer_cast<Student>(user))
                        {
                            file << "Student " << s->getUsername() << " " << s->getPassword() << " " << s->getName() << " " << s->getEmail() << " " << s->getDepartment() << " " << s->ID() << " " << s->getSemester() << endl;
                        }
                        else if (auto t = dynamic_pointer_cast<Teacher>(user))
                        {
                            file << "Teacher " << t->getUsername() << " " << t->getPassword() << " " << t->getName() << " " << t->getEmail() << " " << t->getDepartment() << " " << t->ID() << " " << t->getDesignation() << endl;
                        }
                    }
                    file.close();
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                cout << "User not found.\n";
            }
        }
        else if (choice == 4)
        {
            cout << "\nStudent List:\n";
            cout << string(95, '-') << endl;
            cout << left << setw(5) << "No." << setw(15) << "Username" << setw(15) << "Name" << setw(25) << "Email" << setw(10) << "Dept" << setw(15) << "ID" << setw(10) << "Semester" << endl;
            cout << string(95, '-') << endl;
            int s_count = 0;
            for (auto &user : users)
            {
                if (auto s = dynamic_pointer_cast<Student>(user))
                {
                    s_count++;
                    cout << left << setw(5) << s_count << setw(15) << s->getUsername() << setw(15) << s->getName() << setw(25) << s->getEmail() << setw(10) << s->getDepartment() << setw(15) << s->ID() << setw(10) << s->getSemester() << endl;
                }
            }
            cout << "\nTeacher List:\n";
            cout << string(100, '-') << endl;
            cout << left << setw(5) << "No." << setw(15) << "Username" << setw(15) << "Name" << setw(25) << "Email" << setw(10) << "Dept" << setw(15) << "ID" << setw(15) << "Designation" << endl;
            cout << string(100, '-') << endl;
            int t_count = 0;
            for (auto &user : users)
            {
                if (auto t = dynamic_pointer_cast<Teacher>(user))
                {
                    t_count++;
                    cout << left << setw(5) << t_count << setw(15) << t->getUsername() << setw(15) << t->getName() << setw(25) << t->getEmail() << setw(10) << t->getDepartment() << setw(15) << t->ID() << setw(15) << t->getDesignation() << endl;
                }
            }
            cout << "\nTotal Users: " << (s_count + t_count) << " (Students: " << s_count << ", Teachers: " << t_count << ")\n";
        }
        else if (choice == 5)
        {
            string u;
            cout << "Enter username to edit: ";
            cin >> u;
            bool found = false;
            for (auto &user : users)
            {
                if (user->getUsername() == u)
                {
                    cout << "\nCurrent Information:\n";
                    cout << "Username: " << user->getUsername() << endl;
                    cout << "Name: " << user->getName() << endl;
                    cout << "Email: " << user->getEmail() << endl;
                    cout << "Department: " << user->getDepartment() << endl;

                    string new_password, new_name, new_email, new_dept;
                    // Properly change password with validation
                    inputPassword(new_password);
                    user->setPassword(new_password);

                    cout << "Enter new name: ";
                    cin >> new_name;
                    cout << "Enter new email: ";
                    cin >> new_email;

                    // Validate Department
                    getValidInput(new_dept, "Enter new department (CSE, EEE, MPE, CEE, TVE): ", [](const string &input) -> bool
                                  {
                        if (input == "CSE" || input == "EEE" || input == "MPE" || input == "CEE" || input == "TVE") return true;
                        cout << "Invalid Department. Allowed: CSE, EEE, MPE, CEE, TVE\n";
                        return false; });

                    user->setName(new_name);
                    user->setEmail(new_email);
                    user->setDepartment(new_dept);
                    saveAllUsers();

                    cout << "User information updated.\n";
                    found = true;
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

void Admin::saveAllUsers()
{
    ofstream file("users.txt");
    if (!file.is_open())
    {
        cout << "Error opening users file for saving.\n";
        return;
    }
    for (auto &user : users)
    {
        if (auto s = dynamic_pointer_cast<Student>(user))
        {
            file << "Student " << s->getUsername() << " " << s->getPassword() << " " << s->getName() << " " << s->getEmail() << " " << s->getDepartment() << " " << s->ID() << " " << s->getSemester() << endl;
        }
        else if (auto t = dynamic_pointer_cast<Teacher>(user))
        {
            file << "Teacher " << t->getUsername() << " " << t->getPassword() << " " << t->getName() << " " << t->getEmail() << " " << t->getDepartment() << " " << t->ID() << " " << t->getDesignation() << endl;
        }
    }
    file.close();
}