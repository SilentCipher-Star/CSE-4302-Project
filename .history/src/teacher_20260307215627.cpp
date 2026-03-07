#include "../include/teacher.hpp"

Teacher::Teacher(int id, QString name, QString email, QString dept, QString desig)
    : Person(id, name, email), department(dept), designation(desig)
{
}

// Operator Overloading Implementations
bool Teacher::operator<(const Teacher& other) const
{
    return this->getId() < other.getId();
}

bool Teacher::operator>(const Teacher& other) const
{
    return this->getId() > other.getId();
}

bool Teacher::operator==(const Teacher& other) const
{
    return this->getId() == other.getId();
}

bool Teacher::operator!=(const Teacher& other) const
{
    return !(*this == other);
}