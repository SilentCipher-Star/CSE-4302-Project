#include "../include/appmanager.hpp"
#include "../include/manager_academics.hpp"
#include "../include/manager_notices.hpp"
#include "../include/manager_persons.hpp"
#include "../include/manager_messages.hpp"
#include "../include/csvhandler.hpp"

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

int AcadenceManager::sendBulkGradeReports(int teacherId, int assessmentId, const QString &extraNote)
{
    // Find assessment details
    QVector<Assessment> allAssessments = ManagerAcademics::getAssessments();
    Assessment *target = nullptr;
    for (auto &a : allAssessments)
    {
        if (a.getId() == assessmentId)
        {
            target = &a;
            break;
        }
    }
    if (!target) return 0;

    int courseId = target->getCourseId();
    QString courseName = target->getCourseName();
    QString assessTitle = target->getTitle();
    QString assessType = target->getType();
    int maxMarks = target->getMaxMarks();

    Teacher *teacher = ManagerPersons::getTeacher(teacherId);
    QString teacherName = teacher ? teacher->getName() : "Unknown";
    delete teacher;

    // Get course to find students
    Course *course = ManagerAcademics::getCourse(courseId);
    if (!course) return 0;

    QVector<Student *> students = ManagerAcademics::getStudentsByEnrollment(courseId);
    if (students.isEmpty())
        students = ManagerAcademics::getStudentsBySemester(course->getSemester());
    delete course;

    // Get grades
    QVector<QStringList> gradeData = CsvHandler::readCsv("grades.csv");

    int sentCount = 0;
    for (auto *stu : students)
    {
        int sid = stu->getId();

        // Find this student's grade for this assessment
        double marks = -1;
        for (const auto &row : gradeData)
        {
            if (row.size() >= 3 && row[0].toInt() == sid && row[1].toInt() == assessmentId)
            {
                marks = row[2].toDouble();
                break;
            }
        }

        QString subject = QString("Grade Report: %1 - %2").arg(courseName).arg(assessTitle);

        QString body;
        body += QString("Dear %1,\n\n").arg(stu->getName().trimmed());
        body += QString("This is your personal grade report for:\n");
        body += QString("  Course: %1\n").arg(courseName);
        body += QString("  Assessment: %1 (%2)\n").arg(assessTitle).arg(assessType);
        body += QString("  Max Marks: %1\n\n").arg(maxMarks);

        if (marks >= 0)
        {
            double pct = (maxMarks > 0) ? (marks / maxMarks * 100.0) : 0.0;
            body += QString("  Your Marks: %1 / %2  (%3%)\n")
                        .arg(QString::number(marks, 'f', 1))
                        .arg(maxMarks)
                        .arg(QString::number(pct, 'f', 1));

            if (pct >= 80)
                body += "  Status: Excellent!\n";
            else if (pct >= 60)
                body += "  Status: Good\n";
            else if (pct >= 40)
                body += "  Status: Needs Improvement\n";
            else
                body += "  Status: At Risk - Please consult your instructor\n";
        }
        else
        {
            body += "  Your Marks: Not yet graded\n";
        }

        if (!extraNote.isEmpty())
            body += QString("\nInstructor's Note: %1\n").arg(extraNote);

        body += QString("\n- %1").arg(teacherName);

        ManagerMessages::sendMessage(teacherId, "Teacher", sid, subject, body);
        sentCount++;
    }

    qDeleteAll(students);
    notifyObservers(DataType::Messages);
    return sentCount;
}
