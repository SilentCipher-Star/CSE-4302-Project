#pragma once
#include "../App/User.hpp"
#include <memory>
#include <string>
#include <vector>
#include "Routine/Routine.hpp"

using namespace std;

class Student : public User
{
private:
    string Student_ID;
    int semester;

public:
    Student(string u, string p, string n, string e, string d, string ID, int sem);
    ~Student() = default;
    void showMenu() override;
    string ID() const { return Student_ID; }
    int getSemester() const { return semester; }

    unique_ptr<Routine> routine;

    void viewNotices();
    void viewGrades();
    void checkStudyPlanner();
    void checkHabitTracker();
    void createHabit();
    void viewHabit();
    void deleteHabit();
    void markHabitDone();
};