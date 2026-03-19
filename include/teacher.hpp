#pragma once
#include "person.hpp"

class Teacher : public Person
{
private:
    QString department;
    QString designation;
    double salary;

public:
    Teacher(int id, QString name, QString email, QString dept = "", QString desig = "", double sal = 0.0);

    QString getRole() const override { return "Teacher"; }

    QString getDepartment() const { return department; }
    QString getDesignation() const { return designation; }
    double getSalary() const { return salary; }

    void setSalary(double s) { salary = s; }
    void setDesignation(QString d) { designation = d; }
    void setDepartment(QString d) { department = d; }

    // Operator Overloading
    bool operator<(const Teacher &other) const;
    bool operator==(const Teacher &other) const;
};
