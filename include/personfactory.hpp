#pragma once

#include "admin.hpp"
#include "student.hpp"
#include "teacher.hpp"
#include <memory>

class PersonFactory
{
public:
    static std::unique_ptr<Student> createStudent(int id, const QString &name, const QString &email, const QString &department, const QString &batch, int semester);
    static std::unique_ptr<Teacher> createTeacher(int id, const QString &name, const QString &email, const QString &department, const QString &designation);
    static std::unique_ptr<Admin> createAdmin(int id, const QString &name, const QString &email, const QString &username, const QString &password);
};
