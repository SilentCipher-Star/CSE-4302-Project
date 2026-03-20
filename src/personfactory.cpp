#include "../include/personfactory.hpp"

std::unique_ptr<Student> PersonFactory::createStudent(int id, const QString &name, const QString &email, const QString &department, const QString &batch, int semester)
{
    return std::make_unique<Student>(id, name, email, department, batch, semester);
}

std::unique_ptr<Teacher> PersonFactory::createTeacher(int id, const QString &name, const QString &email, const QString &department, const QString &designation)
{
    return std::make_unique<Teacher>(id, name, email, department, designation);
}

std::unique_ptr<Admin> PersonFactory::createAdmin(int id, const QString &name, const QString &email, const QString &username, const QString &password)
{
    return std::make_unique<Admin>(id, name, email, username, password);
}
