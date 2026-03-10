#pragma once
#include "person.hpp"

class Admin : public Person
{
public:
    Admin(int id, QString name, QString email, QString username, QString password);

    QString getRole() const override;
};
