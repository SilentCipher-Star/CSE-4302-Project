#ifndef ACADENCEMANAGER_HPP
#define ACADENCEMANAGER_HPP

#include <QString>
#include <QVector>
#include <QDate>
#include "student.hpp"
#include "teacher.hpp"
#include "course.hpp"
#include "habit.hpp"
#include "routine.hpp"
#include "exceptions.hpp"
#include "csvhandler.hpp"

class Notice
{
private:
    QString date;
    QString author;
    QString content;

public:
    Notice(QString d, QString a, QString c) : date(d), author(a), content(c) {}
    QString getDate() const { return date; }
    QString getAuthor() const { return author; }
    QString getContent() const { return content; }
};

class Task
{
private:
    int id;
    QString description;
    bool isCompleted;

public:
    Task(int id, QString desc, bool completed) : id(id), description(desc), isCompleted(completed) {}
    int getId() const { return id; }
    QString getDescription() const { return description; }
    bool getIsCompleted() const { return isCompleted; }
    void setCompleted(bool val) { isCompleted = val; }

    bool operator==(const Task &other) const
    {
        return this->id == other.id && this->description == other.description &&
               this->isCompleted == other.isCompleted;
    }
};

class Assessment
{
private:
    int id;
    int courseId;
    QString courseName;
    QString title;
    QString type;
    QString date;
    int maxMarks;

public:
    Assessment(int id, int cid, QString cname, QString t, QString type, QString d, int max)
        : id(id), courseId(cid), courseName(cname), title(t), type(type), date(d), maxMarks(max) {}

    int getId() const { return id; }
    int getCourseId() const { return courseId; }
    QString getCourseName() const { return courseName; }
    QString getTitle() const { return title; }
    QString getType() const { return type; }
    QString getDate() const { return date; }
    int getMaxMarks() const { return maxMarks; }
};

class AttendanceRecord
{
private:
    QString courseName;
    int totalClasses;
    int attendedClasses;
    double totalMarksObtained;
    double totalMaxMarks;

public:
    AttendanceRecord(QString cname, int total, int attended, double marks, double max)
        : courseName(cname), totalClasses(total), attendedClasses(attended), totalMarksObtained(marks), totalMaxMarks(max) {}

    QString getCourseName() const { return courseName; }
    int getTotalClasses() const { return totalClasses; }
    int getAttendedClasses() const { return attendedClasses; }
    double getTotalMarksObtained() const { return totalMarksObtained; }
    double getTotalMaxMarks() const { return totalMaxMarks; }
};

class Query
{
private:
    int id;
    int studentId;
    QString studentName;
    QString question;
    QString answer;

public:
    Query(int id, int sid, QString sname, QString q, QString a)
        : id(id), studentId(sid), studentName(sname), question(q), answer(a) {}

    int getId() const { return id; }
    int getStudentId() const { return studentId; }
    QString getStudentName() const { return studentName; }
    QString getQuestion() const { return question; }
    QString getAnswer() const { return answer; }
};

class AcadenceManager
{
public:
    AcadenceManager();

    QString login(const QString &username, const QString &password, int &userId);
    bool changePassword(int userId, const QString &role, const QString &oldPass, const QString &newPass);

    QVector<Notice> getNotices();
    void addNotice(const QString &content, const QString &author);
    QString getNextClass(int userId);
    QString getDashboardStats(int userId, QString role);

    Student *getStudent(int id);
    Teacher *getTeacher(int id);

    QVector<Task> getTasks(int userId);
    void addTask(int userId, const QString &description);
    void completeTask(int taskId, bool status);

    DailyPrayerStatus getDailyPrayers(int userId, QString date);
    void updateDailyPrayer(int userId, QString date, QString prayer, bool status);
    QVector<Habit *> getHabits(int userId);
    void addHabit(Habit *h);
    void updateHabit(Habit *h);
    void deleteHabit(int id);

    QVector<RoutineSession> getRoutineForDay(QString day, int semester = -1);
    void addRoutineItem(QString day, QString start, QString end, QString code, QString name, QString room, QString instructor, int semester);

    QVector<Course *> getTeacherCourses(int teacherId);
    Course *getCourse(int id);
    QVector<Assessment> getAssessments();
    void addAssessment(int courseId, QString title, QString type, QString date, int maxMarks);

    QVector<AttendanceRecord> getStudentAttendance(int studentId);
    QVector<Student *> getStudentsBySemester(int semester);

    double getGrade(int studentId, int assessmentId);
    void addGrade(int studentId, int assessmentId, double marks);

    QVector<QString> getCourseDates(int courseId);
    bool isPresent(int courseId, int studentId, QString date);
    void markAttendance(int courseId, int studentId, QString date, bool present);

    QVector<Query> getQueries(int userId, QString role);
    void addQuery(int userId, QString question);
    void answerQuery(int queryId, QString answer);
};

template <typename T>
T *findById(const QVector<T *> &list, int id)
{
    for (T *item : list)
    {
        if (item->getId() == id)
            return item;
    }
    return nullptr;
}

#endif // ACADENCEMANAGER_HPP