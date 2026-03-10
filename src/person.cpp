#include "../include/person.hpp"

int Person::personCount = 0;

Person::Person(int id, QString name, QString email, QString username, QString password)
    : id(id), name(name), email(email), username(username), password(password)
{
    personCount++;
}

Person::Person(const Person &other)
    : id(other.id), name(other.name), email(other.email),
      username(other.username), password(other.password)
{
    personCount++;
}

Person &Person::operator=(const Person &other)
{
    if (this == &other)
        return *this;
    id       = other.id;
    name     = other.name;
    email    = other.email;
    username = other.username;
    password = other.password;
    return *this;
}

Person::Person(Person &&other) noexcept
    : id(other.id), name(std::move(other.name)), email(std::move(other.email)),
      username(std::move(other.username)), password(std::move(other.password))
{
    other.id = 0;
}

Person &Person::operator=(Person &&other) noexcept
{
    if (this == &other)
        return *this;
    id       = other.id;
    name     = std::move(other.name);
    email    = std::move(other.email);
    username = std::move(other.username);
    password = std::move(other.password);
    other.id = 0;
    return *this;
}

Person::~Person()
{
    personCount--;
}

int Person::getPersonCount()
{
    return personCount;
}