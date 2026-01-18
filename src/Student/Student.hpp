#pragma once
#include "../App/Person.hpp"

class Student : public Person
{
public:
    Student(std::string u, std::string p, std::string dept, std::string prog, int sem);
    void showMenu() override;

    std::string getDepartment() const { return department; }
    std::string getProgram() const { return program; }
    int getSemester() const { return semester; }

private:
    std::string department;
    std::string program;
    int semester;

    void viewSchedule();
    void viewNotices();
    void viewGrades();
    void checkStudyPlanner();
};