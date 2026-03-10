#pragma once

#include "admin.hpp"
#include "student.hpp"
#include "teacher.hpp"

class PersonFactory
{
public:
    static Student *createStudent(int id, const QString &name, const QString &email, const QString &department, const QString &batch, int semester);
    static Teacher *createTeacher(int id, const QString &name, const QString &email, const QString &department, const QString &designation);
    static Admin *createAdmin(int id, const QString &name, const QString &email, const QString &username, const QString &password);
};
