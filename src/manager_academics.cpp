#include "../include/manager_academics.hpp"
#include "../include/manager_persons.hpp"
#include "../include/personfactory.hpp"
#include "../include/csvhandler.hpp"
#include <QMap>
#include <QSet>
#include <algorithm>

namespace
{
    Course *parseCourse(const QStringList &row)
    {
        if (row.size() >= 6)
        {
            return new Course(row[0].toInt(), row[1], row[2], row[3].toInt(), row[4].toInt(), row[5].toInt());
        }
        return nullptr;
    }
}

// Academics namespace courses implementation
QVector<Course *> ManagerAcademics::getTeacherCourses(int teacherId)
{
    QVector<Course *> courses;
    QVector<QStringList> data = CsvHandler::readCsv("courses.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 6 && row[3].toInt() == teacherId)
            if (auto c = parseCourse(row))
                courses.append(c);
    }
    return courses;
}

Course *ManagerAcademics::getCourse(int id)
{
    QVector<QStringList> data = CsvHandler::readCsv("courses.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 6 && row[0].toInt() == id)
            return parseCourse(row);
    }
    return nullptr;
}

QVector<Assessment> ManagerAcademics::getAssessments()
{
    QVector<Assessment> list;
    QVector<QStringList> assessmentData = CsvHandler::readCsv("assessments.csv");

    // Optimization: Read courses once to create a lookup map
    QVector<QStringList> courseData = CsvHandler::readCsv("courses.csv");
    QMap<int, QString> courseMap;
    for (const auto &row : courseData)
        if (row.size() >= 3)
            courseMap.insert(row[0].toInt(), row[2]);

    for (const auto &row : assessmentData)
    {
        if (row.size() >= 6)
        {
            int courseId = row[1].toInt();
            QString courseName = courseMap.value(courseId, "Unknown");
            list.append(Assessment(row[0].toInt(), courseId, courseName, row[2], row[3], row[4], row[5].toInt()));
        }
    }
    return list;
}

QVector<Assessment> ManagerAcademics::getTeacherAssessments(int teacherId)
{
    QVector<int> courseIds;
    QVector<Course *> courses = getTeacherCourses(teacherId);
    for (auto c : courses)
        courseIds.append(c->getId());
    qDeleteAll(courses);

    QVector<Assessment> result;
    QVector<Assessment> all = getAssessments();
    for (const auto &a : all)
    {
        if (courseIds.contains(a.getCourseId()))
            result.append(a);
    }
    return result;
}

QVector<Assessment> ManagerAcademics::getStudentAssessments(int studentId)
{
    Student *s = ManagerPersons::getStudent(studentId);
    if (!s)
        return {};
    int semester = s->getSemester();
    delete s;

    // Get course IDs for this semester
    QVector<QStringList> courseData = CsvHandler::readCsv("courses.csv");
    QVector<int> courseIds;
    for (const auto &row : courseData)
    {
        if (row.size() >= 6 && row[4].toInt() == semester)
            courseIds.append(row[0].toInt());
    }

    QVector<Assessment> result;
    QVector<Assessment> all = getAssessments();
    for (const auto &a : all)
    {
        if (courseIds.contains(a.getCourseId()))
            result.append(a);
    }
    return result;
}

void ManagerAcademics::addAssessment(int courseId, QString title, QString type, QString date, int maxMarks)
{
    QVector<QStringList> data = CsvHandler::readCsv("assessments.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());

    CsvHandler::appendCsv("assessments.csv", {QString::number(maxId + 1), QString::number(courseId), title, type, date, QString::number(maxMarks)});
}

QVector<Student *> ManagerAcademics::getStudentsByEnrollment(int courseId)
{
    QVector<Student *> list;
    QVector<QStringList> enroll = CsvHandler::readCsv("enrollments.csv");
    for (const auto &row : enroll)
    {
        if (row.size() >= 2 && row[1].toInt() == courseId)
        {
            int sid = row[0].toInt();
            Student *s = ManagerPersons::getStudent(sid);
            if (s)
                list.append(s);
        }
    }
    return list;
}

QVector<Student *> ManagerAcademics::getStudentsBySemester(int semester)
{
    QVector<Student *> list;
    QVector<QStringList> data = CsvHandler::readCsv("students.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 8 && row[7].toInt() == semester)
        {
            Student *s = PersonFactory::createStudent(row[0].toInt(), row[1], row[2], row[5], row[6], row[7].toInt());
            s->setUsername(row[3]);
            s->setPassword(row[4]);

            if (row.size() >= 9)
                s->setDateAdmission(QDate::fromString(row[8], Qt::ISODate));
            if (row.size() >= 10)
                s->setGpa(row[9].toDouble());
            list.append(s);
        }
    }
    return list;
}

QVector<AttendanceRecord> ManagerAcademics::getStudentAttendance(int studentId)
{
    QVector<AttendanceRecord> records;
    Student *s = ManagerPersons::getStudent(studentId);
    if (!s)
        return records;
    int semester = s->getSemester();
    delete s;

    QVector<QStringList> courseData = CsvHandler::readCsv("courses.csv");
    QVector<int> courseIds;
    QMap<int, QString> courseNames;
    for (const auto &row : courseData)
    {
        if (row.size() >= 6 && row[4].toInt() == semester)
        {
            int cid = row[0].toInt();
            courseIds.append(cid);
            courseNames[cid] = row[2];
        }
    }

    QVector<QStringList> attData = CsvHandler::readCsv("attendance.csv");
    QVector<QStringList> gradeData = CsvHandler::readCsv("grades.csv");
    QVector<Assessment> assessments = getAssessments();

    for (int cid : courseIds)
    {
        QSet<QString> uniqueDates;
        int attended = 0;
        for (const auto &row : attData)
        {
            // attendance.csv format: StudentID,CourseID,Date,Status
            if (row.size() >= 4 && row[1].toInt() == cid)
            {
                uniqueDates.insert(row[2]);
                if (row[0].toInt() == studentId && row[3] == "1")
                {
                    attended++;
                }
            }
        }

        int totalClasses = uniqueDates.size();
        int attendedClasses = attended;

        double totalMarksObtained = 0;
        double totalMaxMarks = 0;
        for (const auto &a : assessments)
        {
            if (a.getCourseId() == cid)
            {
                bool isGraded = false;
                double obtained = 0.0;
                for (const auto &grow : gradeData)
                {
                    if (grow.size() >= 3 && grow[0].toInt() == studentId && grow[1].toInt() == a.getId())
                    {
                        obtained = grow[2].toDouble();
                        isGraded = true;
                        break;
                    }
                }
                if (isGraded)
                {
                    totalMaxMarks += a.getMaxMarks();
                    totalMarksObtained += obtained;
                }
            }
        }
        records.append(AttendanceRecord(courseNames[cid], totalClasses, attendedClasses, totalMarksObtained, totalMaxMarks));
    }
    return records;
}

QVector<QString> ManagerAcademics::getCourseDates(int courseId)
{
    QSet<QString> dates;
    QVector<QStringList> data = CsvHandler::readCsv("attendance.csv");
    for (const auto &row : data)
    {
        // Format: StudentID,CourseID,Date,Status
        if (row.size() >= 4 && row[1].toInt() == courseId)
        {
            dates.insert(row[2]);
        }
    }
    QVector<QString> list = dates.values();
    std::sort(list.begin(), list.end());
    return list;
}

bool ManagerAcademics::isPresent(int courseId, int studentId, QString date)
{
    QVector<QStringList> data = CsvHandler::readCsv("attendance.csv");
    for (const auto &row : data)
    {
        // Format: StudentID,CourseID, Date,Status
        if (row.size() >= 4 && row[0].toInt() == studentId && row[1].toInt() == courseId && row[2] == date)
        {
            return row[3] == "1";
        }
    }
    return false;
}

void ManagerAcademics::markAttendance(int courseId, int studentId, QString date, bool present)
{
    QVector<QStringList> data = CsvHandler::readCsv("attendance.csv");
    bool found = false;
    for (auto &row : data)
    {
        // Format: StudentID,CourseID,Date,Status
        if (row.size() >= 4 && row[0].toInt() == studentId && row[1].toInt() == courseId && row[2] == date)
        {
            row[3] = present ? "1" : "0";
            found = true;
        }
    }
    if (!found)
    {
        data.append({QString::number(studentId), QString::number(courseId), date, present ? "1" : "0"});
    }
    CsvHandler::writeCsv("attendance.csv", data);
}

double ManagerAcademics::getGrade(int studentId, int assessmentId)
{
    QVector<QStringList> data = CsvHandler::readCsv("grades.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 3 && row[0].toInt() == studentId && row[1].toInt() == assessmentId)
        {
            return row[2].toDouble();
        }
    }
    return -1.0;
}

void ManagerAcademics::addGrade(int studentId, int assessmentId, double marks)
{
    QVector<QStringList> data = CsvHandler::readCsv("grades.csv");
    bool found = false;
    for (auto &row : data)
    {
        if (row.size() >= 3 && row[0].toInt() == studentId && row[1].toInt() == assessmentId)
        {
            row[2] = QString::number(marks);
            found = true;
        }
    }
    if (!found)
    {
        data.append({QString::number(studentId), QString::number(assessmentId), QString::number(marks)});
    }
    CsvHandler::writeCsv("grades.csv", data);
}

QMap<int, double> ManagerAcademics::getGradesForAssessment(int assessmentId)
{
    QMap<int, double> grades;
    QVector<QStringList> data = CsvHandler::readCsv("grades.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 3 && row[1].toInt() == assessmentId)
        {
            grades.insert(row[0].toInt(), row[2].toDouble());
        }
    }
    return grades;
}

QSet<QString> ManagerAcademics::getPresenceSet(int courseId)
{
    QSet<QString> presence;
    QVector<QStringList> data = CsvHandler::readCsv("attendance.csv");
    for (const auto &row : data)
    {
        // Format: StudentID,CourseID,Date,Status
        if (row.size() >= 4 && row[1].toInt() == courseId && row[3] == "1")
        {
            // Key: "studentId_date"
            presence.insert(row[0] + "_" + row[2]);
        }
    }
    return presence;
}

QVector<AttendanceAnalytics> ManagerAcademics::getLowAttendanceStudents(int courseId, double threshold)
{
    QVector<AttendanceAnalytics> result;

    Course *course = getCourse(courseId);
    if (!course)
        return result;

    QString courseName = course->getName();
    int semester = course->getSemester();
    delete course;

    QVector<Student *> students = getStudentsBySemester(semester);
    QVector<QStringList> attData = CsvHandler::readCsv("attendance.csv");

    QSet<QString> uniqueDates;
    for (const auto &row : attData)
    {
        if (row.size() >= 4 && row[1].toInt() == courseId)
            uniqueDates.insert(row[2]);
    }
    int totalClasses = uniqueDates.size();

    if (totalClasses == 0)
    {
        qDeleteAll(students);
        return result;
    }

    for (auto *s : students)
    {
        int attended = 0;
        for (const auto &row : attData)
        {
            if (row.size() >= 4 && row[0].toInt() == s->getId() &&
                row[1].toInt() == courseId && row[3] == "1")
            {
                attended++;
            }
        }

        double pct = (double)attended / totalClasses * 100.0;
        if (pct < threshold)
        {
            AttendanceAnalytics analytics;
            analytics.studentId = s->getId();
            analytics.studentName = s->getName();
            analytics.courseId = courseId;
            analytics.courseName = courseName;
            analytics.totalClasses = totalClasses;
            analytics.attendedClasses = attended;
            analytics.percentage = pct;
            result.append(analytics);
        }
    }

    qDeleteAll(students);
    return result;
}

double ManagerAcademics::getOverallAttendancePercentage(int studentId)
{
    Student *s = ManagerPersons::getStudent(studentId);
    if (!s)
        return 0.0;
    int semester = s->getSemester();
    delete s;

    QVector<QStringList> courseData = CsvHandler::readCsv("courses.csv");
    QVector<int> courseIds;
    for (const auto &row : courseData)
    {
        if (row.size() >= 6 && row[4].toInt() == semester)
            courseIds.append(row[0].toInt());
    }

    if (courseIds.isEmpty())
        return 0.0;

    QVector<QStringList> attData = CsvHandler::readCsv("attendance.csv");

    double totalPct = 0.0;
    int coursesWithClasses = 0;

    for (int cid : courseIds)
    {
        QSet<QString> uniqueDates;
        int attended = 0;

        for (const auto &row : attData)
        {
            if (row.size() >= 4 && row[1].toInt() == cid)
            {
                uniqueDates.insert(row[2]);
                if (row[0].toInt() == studentId && row[3] == "1")
                    attended++;
            }
        }

        int total = uniqueDates.size();
        if (total > 0)
        {
            totalPct += (double)attended / total * 100.0;
            coursesWithClasses++;
        }
    }

    if (coursesWithClasses == 0)
        return 0.0;

    return totalPct / coursesWithClasses;
}
