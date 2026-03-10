#ifndef MANAGER_PERSONS_HPP
#define MANAGER_PERSONS_HPP

#include <QString>
#include <QPair>
#include "student.hpp"
#include "teacher.hpp"

class ManagerPersons
{
public:
    static Student *getStudent(int id);
    static Teacher *getTeacher(int id);
    static QPair<QString, QString> getAdminProfile(int id);
};

#endif // MANAGER_PERSONS_HPP
