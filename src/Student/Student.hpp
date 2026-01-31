#pragma once
#include "../App/User.hpp"
#include <memory>
#include <string>
#include <vector>
#include "Routine/Routine.hpp"
#include "Habit_Tracker/Habit_Tracker.hpp"

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
    unique_ptr<HabitTracker> habitTracker;

    void changePassword();
    void viewNotices();
    void viewGrades();
    void checkStudyPlanner();
};