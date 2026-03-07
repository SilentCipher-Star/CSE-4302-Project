#include "../include/manager_persons.hpp"
#include "../include/csvhandler.hpp"
#include "../include/personfactory.hpp"

template <typename T>
class CsvRepository
{
public:
    explicit CsvRepository(const QString &filename) : m_filename(filename) {}

    template <typename Func>
    T *findById(int id, Func parser)
    {
        QVector<QStringList> data = CsvHandler::readCsv(m_filename);
        for (const auto &row : data)
        {
            if (!row.isEmpty() && row[0].toInt() == id)
            {
                return parser(row);
            }
        }
        return nullptr;
    }

private:
    QString m_filename;
};

Student *ManagerPersons::getStudent(int id)
{
    CsvRepository<Student> repo("students.csv");
    return repo.findById(id, [](const QStringList &row) -> Student *
                         {
        if (row.size() < 8) return nullptr;
    Student *s = PersonFactory::createStudent(row[0].toInt(), row[1], row[2], row[5], row[6], row[7].toInt());
        s->setUsername(row[3]);
        s->setPassword(row[4]);

        if (row.size() >= 9)
            s->setDateAdmission(QDate::fromString(row[8], Qt::ISODate));
        if (row.size() >= 10)
            s->setGpa(row[9].toDouble());

        return s; });
}

Teacher *ManagerPersons::getTeacher(int id)
{
    CsvRepository<Teacher> repo("teachers.csv");
    return repo.findById(id, [](const QStringList &row) -> Teacher *
                         {
        if (row.size() < 7) return nullptr;
    Teacher *t = PersonFactory::createTeacher(row[0].toInt(), row[1], row[2], row[5], row[6]);
        t->setUsername(row[3]);
        t->setPassword(row[4]);

        if (row.size() >= 8)
            t->setSalary(row[7].toDouble());

        return t; });
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
