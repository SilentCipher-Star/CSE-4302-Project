#pragma once
#include <QString>
#include <QDate>

class QTableView;
class QStandardItemModel;

class Utils
{
public:
    static QString validatePassword(const QString &password);
    static QString validateUsername(const QString &username);
    static void loadFonts();
    static QDate getDateForDay(QString dayName);
    static void saveTableData(QStandardItemModel *model, const QString &tableName);
};

namespace Constants
{
    namespace Role
    {
        inline const QString Admin = "Admin";
        inline const QString Student = "Student";
        inline const QString Teacher = "Teacher";
    }
    namespace Table
    {
        inline const QString Admins = "admins";
        inline const QString Students = "students";
        inline const QString Teachers = "teachers";
        inline const QString Courses = "courses";
        inline const QString Routine = "routine";
        inline const QString RoutineAdj = "routine_adjustments";
        inline const QString Notices = "notices";
    }
}
