#ifndef MANAGER_ACADEMICS_HPP
#define MANAGER_ACADEMICS_HPP

#include <QString>
#include <QVector>
#include "course.hpp"
#include "appmanager.hpp"

class ManagerAcademics
{
public:
    static QVector<Course *> getTeacherCourses(int teacherId);
    static Course *getCourse(int id);

    static QVector<Assessment> getAssessments();
    static QVector<Assessment> getTeacherAssessments(int teacherId);
    static QVector<Assessment> getStudentAssessments(int studentId);
    static void addAssessment(int courseId, QString title, QString type, QString date, int maxMarks);

    static QVector<Student *> getStudentsByEnrollment(int courseId);
    static QVector<Student *> getStudentsBySemester(int semester);

    static QVector<AttendanceRecord> getStudentAttendance(int studentId);
    static QVector<QString> getCourseDates(int courseId);
    static bool isPresent(int courseId, int studentId, QString date);
    static void markAttendance(int courseId, int studentId, QString date, bool present);

    static double getGrade(int studentId, int assessmentId);
    static void addGrade(int studentId, int assessmentId, double marks);
};

#endif // MANAGER_ACADEMICS_HPP
