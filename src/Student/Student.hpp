#pragma once
#include "../App/Person.hpp"

class Student : public Person
{
public:
    Student(std::string u, std::string p);
    void showMenu() override;

private:
    void viewSchedule();
    void viewNotices();
    void viewGrades();
    void checkStudyPlanner();
};