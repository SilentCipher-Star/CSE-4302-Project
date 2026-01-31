#pragma once
#include "../App/User.hpp"

using namespace std;

class Teacher : public User
{
private:
    string Teacher_ID;
    string designation;

public:
    Teacher(string u, string p, string n, string e, string d, string ID, string desig);
    ~Teacher() = default;
    void showMenu() override;

    string ID() const { return Teacher_ID; }
    string getDesignation() const { return designation; }

    void postNotice();
    void assignGrade();
};