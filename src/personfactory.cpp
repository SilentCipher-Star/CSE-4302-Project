#include "../include/personfactory.hpp"

Student *PersonFactory::createStudent(int id, const QString &name, const QString &email, const QString &department, const QString &batch, int semester)
{
    return new Student(id, name, email, department, batch, semester);
}

Teacher *PersonFactory::createTeacher(int id, const QString &name, const QString &email, const QString &department, const QString &designation)
{
    return new Teacher(id, name, email, department, designation);
}

Admin *PersonFactory::createAdmin(int id, const QString &name, const QString &email, const QString &username, const QString &password)
{
    return new Admin(id, name, email, username, password);
}
