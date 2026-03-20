#pragma once

#include <QString>
#include <QPair>
#include "student.hpp"
#include "teacher.hpp"
#include <memory>

class ManagerPersons
{
public:
    static std::unique_ptr<Student> getStudent(int id);
    static std::unique_ptr<Teacher> getTeacher(int id);
    static QPair<QString, QString> getAdminProfile(int id);
};
