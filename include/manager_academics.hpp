#pragma once

#include <QString>
#include <QVector>
#include <QMap>
#include <QSet>
#include <memory>
#include <vector>
#include "course.hpp"
#include "appmanager.hpp"

class ManagerAcademics
{
public:
    struct AttendanceUpdate
    {
        int studentId;
        QString date;
        bool present;
    };
    struct GradeUpdate
    {
        int studentId;
        double marks;
    };

    static std::vector<std::unique_ptr<Course>> getTeacherCourses(int teacherId);
    static std::unique_ptr<Course> getCourse(int id);

    static QVector<Assessment> getAssessments();
    static QVector<Assessment> getTeacherAssessments(int teacherId);
    static QVector<Assessment> getStudentAssessments(int studentId);
    static void addAssessment(int courseId, QString title, QString type, QString date, int maxMarks);

    static std::vector<std::unique_ptr<Student>> getStudentsByEnrollment(int courseId);
    static std::vector<std::unique_ptr<Student>> getStudentsBySemester(int semester);

    static QVector<AttendanceRecord> getStudentAttendance(int studentId);
    static QVector<QString> getCourseDates(int courseId);
    static bool isPresent(int courseId, int studentId, QString date);
    static void markAttendance(int courseId, int studentId, QString date, bool present);
    static void markAttendanceBatch(int courseId, const QVector<AttendanceUpdate> &updates);

    static double getGrade(int studentId, int assessmentId);
    static void addGrade(int studentId, int assessmentId, double marks);
    static void addGradeBatch(int assessmentId, const QVector<GradeUpdate> &updates);

    static QMap<int, double> getGradesForAssessment(int assessmentId);
    static QSet<QString> getPresenceSet(int courseId);

    static QVector<AttendanceAnalytics> getLowAttendanceStudents(int courseId, double threshold = 75.0);
    static double getOverallAttendancePercentage(int studentId);
};
