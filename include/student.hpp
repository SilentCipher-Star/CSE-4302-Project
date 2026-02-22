#ifndef STUDENT_H
#define STUDENT_H

#include "person.hpp"
#include <QDate>
#include <QMap>
#include <QList>

class Student : public Person
{
private:
    QString department;
    QString batch;
    int semester;
    double m_gpa;
    QDate dateAdmission;

public:
    Student(int id, QString name, QString email, QString dept = "", QString batch = "", int sem = 1);

    QString getRole() const override { return "Student"; }

    QString getDepartment() const { return department; }
    QString getBatch() const { return batch; }
    int getSemester() const { return semester; }
    QDate getDateAdmission() const { return dateAdmission; }
    double getCGPA() const { return m_gpa; }

    void setGpa(double gpa) { m_gpa = gpa; }
    void setDateAdmission(const QDate &date) { dateAdmission = date; }

    double calculateGPA() const;
};

#endif // STUDENT_H