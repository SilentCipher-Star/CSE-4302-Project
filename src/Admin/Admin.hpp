#pragma once
#include "../App/Person.hpp"
#include <iostream>

class Admin : public Person
{
public:
    Admin(std::string u, std::string p) : Person(u, p)
    {
        role = "Admin";
    }

    void showMenu() override
    {
        std::cout << "Logged in as Admin." << std::endl;
    }
};