#pragma once
#include "../App/Person.hpp"

class Teacher : public Person
{
public:
    Teacher(std::string u, std::string p, std::string desig, std::string dept);
    void showMenu() override;

    std::string getDesignation() const { return designation; }
    std::string getDepartment() const { return department; }

private:
    std::string designation;
    std::string department;

    void postNotice();
    void assignGrade();
};