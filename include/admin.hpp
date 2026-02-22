#ifndef ADMIN_H
#define ADMIN_H

#include "person.hpp"

class Admin : public Person
{
public:
    Admin(int id, QString name, QString email, QString username, QString password);

    QString getRole() const override;
};

#endif // ADMIN_H