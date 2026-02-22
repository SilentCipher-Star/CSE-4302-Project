#ifndef TEACHER_H
#define TEACHER_H

#include "person.hpp"

class Teacher : public Person
{
private:
    QString department;
    QString designation;
    double salary;

public:
    Teacher(int id, QString name, QString email, QString dept = "", QString desig = "");

    QString getRole() const override { return "Teacher"; }

    QString getDepartment() const { return department; }
    QString getDesignation() const { return designation; }
    double getSalary() const { return salary; }

    void setSalary(double s) { salary = s; }
};

#endif // TEACHER_H