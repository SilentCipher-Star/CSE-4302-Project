#include "../include/student.hpp"
#include <cmath>

Student::Student(int id, QString name, QString email, QString dept, QString batch, int sem)
    : Person(id, name, email), department(dept), batch(batch), semester(sem), m_gpa(0.0)
{
}

double Student::calculateGPA() const
{
    return m_gpa;
}