#include "../include/appmanager.hpp"
#include "../include/manager_academics.hpp"
#include "../include/manager_notices.hpp"
#include "../include/manager_persons.hpp"

QVector<AttendanceAnalytics> AcadenceManager::getLowAttendanceStudents(int courseId, double threshold)
{
    return ManagerAcademics::getLowAttendanceStudents(courseId, threshold);
}

double AcadenceManager::getOverallAttendancePercentage(int studentId)
{
    return ManagerAcademics::getOverallAttendancePercentage(studentId);
}

int AcadenceManager::generateAttendanceWarnings(int courseId, int teacherId, double threshold)
{
    QVector<AttendanceAnalytics> flagged = ManagerAcademics::getLowAttendanceStudents(courseId, threshold);

    if (flagged.isEmpty())
        return 0;

    Teacher *teacher = ManagerPersons::getTeacher(teacherId);
    QString teacherName = teacher ? teacher->getName() : "Unknown";
    delete teacher;

    for (const auto &a : flagged)
    {
        QString severity = (a.percentage < 60.0) ? "CRITICAL" : "WARNING";
        QString content = QString("[%1] %2 (ID: %3) has %4% attendance in %5 (%6/%7 classes). Minimum required: %8%")
                              .arg(severity)
                              .arg(a.studentName)
                              .arg(a.studentId)
                              .arg(QString::number(a.percentage, 'f', 1))
                              .arg(a.courseName)
                              .arg(a.attendedClasses)
                              .arg(a.totalClasses)
                              .arg(QString::number(threshold, 'f', 0));
        ManagerNotices::addNotice(content, teacherName);
    }

    notifyObservers(DataType::Notices);
    return flagged.size();
}
