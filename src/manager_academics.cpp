#include "../include/manager_academics.hpp"
#include "../include/manager_persons.hpp"
#include "../include/personfactory.hpp"
#include "../include/csvhandler.hpp"
#include <QMap>
#include <QSet>
#include <algorithm>

namespace
{
    std::unique_ptr<Course> parseCourse(const QStringList &row)
    {
        if (row.size() >= 6)
        {
            return std::make_unique<Course>(row[0].toInt(), row[1], row[2], row[3].toInt(), row[4].toInt(), row[5].toInt());
        }
        return nullptr;
    }
}

// Academics namespace courses implementation
std::vector<std::unique_ptr<Course>> ManagerAcademics::getTeacherCourses(int teacherId)
{
    std::vector<std::unique_ptr<Course>> courses;
    QVector<QStringList> data = CsvHandler::readCsv("courses.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 6 && row[3].toInt() == teacherId)
            if (auto c = parseCourse(row))
                courses.push_back(std::move(c));
    }
    return courses;
}

std::unique_ptr<Course> ManagerAcademics::getCourse(int id)
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
    auto courses = getTeacherCourses(teacherId);
    for (const auto &c : courses)
        courseIds.append(c->getId());

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
    auto s = ManagerPersons::getStudent(studentId);
    if (!s)
        return {};
    int semester = s->getSemester();

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

std::vector<std::unique_ptr<Student>> ManagerAcademics::getStudentsByEnrollment(int courseId)
{
    std::vector<std::unique_ptr<Student>> list;
    QVector<QStringList> enroll = CsvHandler::readCsv("enrollments.csv");
    for (const auto &row : enroll)
    {
        if (row.size() >= 2 && row[1].toInt() == courseId)
        {
            int sid = row[0].toInt();
            auto s = ManagerPersons::getStudent(sid);
            if (s)
                list.push_back(std::move(s));
        }
    }
    return list;
}

std::vector<std::unique_ptr<Student>> ManagerAcademics::getStudentsBySemester(int semester)
{
    std::vector<std::unique_ptr<Student>> list;
    QVector<QStringList> data = CsvHandler::readCsv("students.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 8 && row[7].toInt() == semester)
        {
            auto s = PersonFactory::createStudent(row[0].toInt(), row[1], row[2], row[5], row[6], row[7].toInt());
            s->setUsername(row[3]);
            s->setPassword(row[4]);

            if (row.size() >= 9)
                s->setDateAdmission(QDate::fromString(row[8], Qt::ISODate));
            if (row.size() >= 10)
                s->setGpa(row[9].toDouble());
            list.push_back(std::move(s));
        }
    }
    return list;
}

QVector<AttendanceRecord> ManagerAcademics::getStudentAttendance(int studentId)
{
    QVector<AttendanceRecord> records;
    auto s = ManagerPersons::getStudent(studentId);
    if (!s)
        return records;
    int semester = s->getSemester();

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

    QMap<int, QSet<QString>> uniqueDatesMap;
    QMap<int, int> attendedMap;
    for (const auto &row : attData)
    {
        if (row.size() >= 4)
        {
            int rowCid = row[1].toInt();
            if (courseIds.contains(rowCid))
            {
                uniqueDatesMap[rowCid].insert(row[2]);
                if (row[0].toInt() == studentId && row[3] == "1")
                    attendedMap[rowCid]++;
            }
        }
    }

    QMap<int, double> studentGrades;
    for (const auto &grow : gradeData)
    {
        if (grow.size() >= 3 && grow[0].toInt() == studentId)
        {
            studentGrades[grow[1].toInt()] = grow[2].toDouble();
        }
    }

    for (int cid : courseIds)
    {
        int totalClasses = uniqueDatesMap[cid].size();
        int attendedClasses = attendedMap.value(cid, 0);

        double totalMarksObtained = 0;
        double totalMaxMarks = 0;
        for (const auto &a : assessments)
        {
            if (a.getCourseId() == cid)
            {
                if (studentGrades.contains(a.getId()))
                {
                    totalMaxMarks += a.getMaxMarks();
                    totalMarksObtained += studentGrades[a.getId()];
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

void ManagerAcademics::markAttendanceBatch(int courseId, const QVector<AttendanceUpdate> &updates)
{
    QVector<QStringList> data = CsvHandler::readCsv("attendance.csv");

    QMap<QString, bool> updateMap;
    for (const auto &u : updates)
    {
        updateMap[QString::number(u.studentId) + "_" + u.date] = u.present;
    }

    for (auto &row : data)
    {
        if (row.size() >= 4 && row[1].toInt() == courseId)
        {
            QString key = row[0] + "_" + row[2];
            if (updateMap.contains(key))
            {
                row[3] = updateMap[key] ? "1" : "0";
                updateMap.remove(key); // Processed
            }
        }
    }

    for (auto it = updateMap.begin(); it != updateMap.end(); ++it)
    {
        QStringList parts = it.key().split("_");
        if (parts.size() == 2)
        {
            data.append({parts[0], QString::number(courseId), parts[1], it.value() ? "1" : "0"});
        }
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

void ManagerAcademics::addGradeBatch(int assessmentId, const QVector<GradeUpdate> &updates)
{
    QVector<QStringList> data = CsvHandler::readCsv("grades.csv");
    QVector<QStringList> newData;

    QMap<int, double> updateMap;
    for (const auto &u : updates)
    {
        updateMap[u.studentId] = u.marks;
    }

    for (const auto &row : data)
    {
        if (row.size() >= 3 && row[1].toInt() == assessmentId)
        {
            int sid = row[0].toInt();
            if (updateMap.contains(sid))
            {
                double m = updateMap[sid];
                updateMap.remove(sid);
                if (m >= 0)
                    newData.append({row[0], row[1], QString::number(m)});
            }
            else
            {
                newData.append(row);
            }
        }
        else
        {
            newData.append(row);
        }
    }

    for (auto it = updateMap.begin(); it != updateMap.end(); ++it)
    {
        if (it.value() >= 0)
            newData.append({QString::number(it.key()), QString::number(assessmentId), QString::number(it.value())});
    }
    CsvHandler::writeCsv("grades.csv", newData);
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

    auto course = getCourse(courseId);
    if (!course)
        return result;

    QString courseName = course->getName();
    int semester = course->getSemester();

    auto students = getStudentsBySemester(semester);
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
        return result;
    }

    QMap<int, int> attendanceCount;
    for (const auto &row : attData)
    {
        if (row.size() >= 4 && row[1].toInt() == courseId && row[3] == "1")
            attendanceCount[row[0].toInt()]++;
    }

    for (const auto &s : students)
    {
        int attended = attendanceCount.value(s->getId(), 0);
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

    return result;
}

double ManagerAcademics::getOverallAttendancePercentage(int studentId)
{
    auto s = ManagerPersons::getStudent(studentId);
    if (!s)
        return 0.0;
    int semester = s->getSemester();

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

    QMap<int, QSet<QString>> uniqueDatesMap;
    QMap<int, int> attendedMap;
    for (const auto &row : attData)
    {
        if (row.size() >= 4)
        {
            int rowCid = row[1].toInt();
            if (courseIds.contains(rowCid))
            {
                uniqueDatesMap[rowCid].insert(row[2]);
                if (row[0].toInt() == studentId && row[3] == "1")
                    attendedMap[rowCid]++;
            }
        }
    }

    double totalPct = 0.0;
    int coursesWithClasses = 0;

    for (int cid : courseIds)
    {
        int total = uniqueDatesMap[cid].size();
        int attended = attendedMap.value(cid, 0);
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
