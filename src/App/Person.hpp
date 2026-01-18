#pragma once
#include <string>
#include <iostream>

class Person
{
protected:
    std::string username;
    std::string password;

    std::string role;
    std::string name;
    std::string email;

public:
    Person(std::string u, std::string p) : username(u), password(p) {}
    virtual ~Person() = default;

    std::string getUsername() const { return username; }
    bool authenticate(const std::string &p) const { return password == p; }
    virtual void showMenu() = 0;
};