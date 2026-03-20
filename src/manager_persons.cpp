#include "../include/manager_persons.hpp"
#include "../include/csvhandler.hpp"
#include "../include/personfactory.hpp"
#include "../include/appmanager.hpp"
#include <unordered_map>

namespace
{
    std::unordered_map<int, std::unique_ptr<Student>> g_studentCache;
    std::unordered_map<int, std::unique_ptr<Teacher>> g_teacherCache;
    int g_lastStudentCount = 0;
    int g_lastTeacherCount = 0;

    std::unique_ptr<Student> parseStudent(const QStringList &row)
    {
        if (row.size() < 8)
            return nullptr;
        auto s = PersonFactory::createStudent(row[0].toInt(), row[1], row[2], row[5], row[6], row[7].toInt());
        s->setUsername(row[3]);
        s->setPassword(row[4]);

        if (row.size() >= 9)
            s->setDateAdmission(QDate::fromString(row[8], Qt::ISODate));
        if (row.size() >= 10)
            s->setGpa(row[9].toDouble());

        return s;
    }

    std::unique_ptr<Teacher> parseTeacher(const QStringList &row)
    {
        if (row.size() < 7)
            return nullptr;
        auto t = PersonFactory::createTeacher(row[0].toInt(), row[1], row[2], row[5], row[6]);
        t->setUsername(row[3]);
        t->setPassword(row[4]);

        if (row.size() >= 8)
            t->setSalary(row[7].toDouble());

        return t;
    }
}

std::unique_ptr<Student> ManagerPersons::getStudent(int id)
{
    // Invalidate cache if the underlying data size has changed
    int currentSize = CsvHandler::readCsv("students.csv").size();
    if (currentSize != g_lastStudentCount)
    {
        g_studentCache.clear();
        g_lastStudentCount = currentSize;
    }

    if (g_studentCache.find(id) != g_studentCache.end())
        return std::make_unique<Student>(*g_studentCache[id]);

    CsvRepository<Student> repo("students.csv");
    auto s = repo.findById(id, parseStudent);
    if (s)
        g_studentCache[id] = std::make_unique<Student>(*s);
    return std::move(s);
}

std::unique_ptr<Teacher> ManagerPersons::getTeacher(int id)
{
    // Invalidate cache if the underlying data size has changed
    int currentSize = CsvHandler::readCsv("teachers.csv").size();
    if (currentSize != g_lastTeacherCount)
    {
        g_teacherCache.clear();
        g_lastTeacherCount = currentSize;
    }

    if (g_teacherCache.find(id) != g_teacherCache.end())
        return std::make_unique<Teacher>(*g_teacherCache[id]);

    CsvRepository<Teacher> repo("teachers.csv");
    auto t = repo.findById(id, parseTeacher);
    if (t)
        g_teacherCache[id] = std::make_unique<Teacher>(*t);
    return std::move(t);
}

QPair<QString, QString> ManagerPersons::getAdminProfile(int id)
{
    // admins.csv: ID,Username,Password,Name,Email
    QVector<QStringList> data = CsvHandler::readCsv("admins.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 5 && row[0].toInt() == id)
        {
            return {row[3], row[4]};
        }
    }
    return {"System Admin", "admin@school.edu"};
}
