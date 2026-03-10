#include "../include/student.hpp"
#include <cmath>


int Student::totalStudents = 0;
double Student::totalCumulativeCGPA = 0.0;

Student::Student(int id, QString name, QString email, QString dept, QString batch, int sem)
    : Person(id, name, email), department(dept), batch(batch), semester(sem), m_gpa(0.0)
{
    ++totalStudents;
}

double Student::calculateGPA() const
{
    return m_gpa;
}

// Operator Overloading Implementations
bool Student::operator<(const Student& other) const
{
    return this->m_gpa < other.m_gpa;
}

bool Student::operator>(const Student& other) const
{
    return this->m_gpa > other.m_gpa;
}

bool Student::operator==(const Student& other) const
{
    return this->getId() == other.getId();
}

bool Student::operator!=(const Student& other) const
{
    return !(*this == other);
}

// Static method implementations
int Student::getTotalStudents()
{
    return totalStudents;
}

double Student::getAverageInstituteCGPA()
{
    if (totalStudents == 0) return 0.0;
    return totalCumulativeCGPA / totalStudents;
}

void Student::incrementStudentCount()
{
    totalStudents++;
}