#pragma once
#include <iostream>
#include <string>
#include "Person.hpp"
using namespace std;

class User : public Person
{
protected:
    string name;
    string email;
    string department;

public:
    User(string u, string p, string n, string e, string d)
        : Person(u, p, "User"), name(n), email(e), department(d) {}
    virtual ~User() = default;

    string getName() const { return name; }
    string getEmail() const { return email; }
    string getDepartment() const { return department; }

    void setName(const string &n) { name = n; }
    void setEmail(const string &e) { email = e; }
    void setDepartment(const string &d) { department = d; }

    virtual void showMenu() = 0;
};
