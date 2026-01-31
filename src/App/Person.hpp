#pragma once
#include <string>
#include <iostream>
using namespace std;

class Person
{
protected:
    string username;
    string password;
    string role;

public:
    Person(string u, string p, string r) : username(u), password(p), role(r) {}
    virtual ~Person() = default;

    string getUsername() const { return username; }
    string getPassword() const { return password; }
    string getRole() const { return role; }
    void setPassword(const string &p) { password = p; }

    bool authenticate(const string &p) const { return password == p; }
    virtual void showMenu() = 0;
};