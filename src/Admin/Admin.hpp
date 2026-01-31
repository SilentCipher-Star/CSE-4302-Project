#pragma once
#include "../App/Person.hpp"
#include <iostream>

class Admin : public Person
{
public:
    Admin();
    void showMenu();
};