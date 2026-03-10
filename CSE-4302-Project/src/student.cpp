#include "../include/student.hpp"
#include <cmath>


int Student::totalStudents = 0;
double Student::totalCumulativeCGPA = 0.0;

Student::Student(int id, QString name, QString email, QString dept, QString batch, int sem)
    : Person(id, name, email), department(dept), batch(batch), semester(sem), m_gpa(0.0)
{
    incrementStudentCount();
    totalCumulativeCGPA += m_gpa;
}

Student::Student(const Student &other)
    : Person(other), department(other.department), batch(other.batch),
      semester(other.semester), m_gpa(other.m_gpa), dateAdmission(other.dateAdmission)
{
}

Student &Student::operator=(const Student &other)
{
    if (this == &other)
        return *this;
    Person::operator=(other);
    department    = other.department;
    batch         = other.batch;
    semester      = other.semester;
    m_gpa         = other.m_gpa;
    dateAdmission = other.dateAdmission;
    return *this;
}

Student::Student(Student &&other) noexcept
    : Person(std::move(other)), department(std::move(other.department)),
      batch(std::move(other.batch)), semester(other.semester),
      m_gpa(other.m_gpa), dateAdmission(std::move(other.dateAdmission))
{
    other.semester = 0;
    other.m_gpa    = 0.0;
}

Student &Student::operator=(Student &&other) noexcept
{
    if (this == &other)
        return *this;
    Person::operator=(std::move(other));
    department    = std::move(other.department);
    batch         = std::move(other.batch);
    semester      = other.semester;
    m_gpa         = other.m_gpa;
    dateAdmission = std::move(other.dateAdmission);
    other.semester = 0;
    other.m_gpa    = 0.0;
    return *this;
}

Student Student::clone() const
{
    return Student(*this);
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