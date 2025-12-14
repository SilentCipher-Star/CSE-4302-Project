#pragma once
#include "../App/Person.hpp"

class Teacher : public Person
{
public:
    Teacher(std::string u, std::string p);
    void showMenu() override;

private:
    void postNotice();
    void assignGrade();
};