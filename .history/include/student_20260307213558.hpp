#pragma once
#include "person.hpp"
#include <QDate>
#include <QMap>
#include <QList>

class Student : public Person
{
private:
    static int totalStudents;
    static double totalCumulativeCGPA;
    
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
    void setSemester(int sem) { semester = sem; }
    void setDepartment(const QString &dept) { department = dept; }
    void setBatch(const QString &batch) { this->batch = batch; }

    // Static methods
    static int getTotalStudents();
    static double getAverageInstituteCGPA();
    static void incrementStudentCount();
    
    double calculateGPA() const;

    // Operator Overloading
    bool operator<(const Student& other) const;
    bool operator>(const Student& other) const;
    bool operator==(const Student& other) const;
    bool operator!=(const Student& other) const;
};
