#pragma once

#include <QString>
#include <QVector>
#include <QDate>
#include <QDebug>
#include <memory>
#include <vector>
#include "student.hpp"
#include "teacher.hpp"
#include "course.hpp"
#include "habit.hpp"
#include "routine.hpp"
#include "exceptions.hpp"
#include "csvhandler.hpp"
#include "command_history.hpp"
#include "models.hpp"

enum class DataType
{
    None,
    Habits,
    Tasks,
    Routine,
    Notices,
    Academics,
    Queries,
    Profile,
    Messages,
    LostFound
};

class IDataObserver
{
public:
    virtual void onDataChanged(DataType type) = 0;
    virtual ~IDataObserver() = default;
};

class AcadenceManager
{
public:
    AcadenceManager();

    // Dispatch logic to respective inner manager layers
    QString login(const QString &username, const QString &password, int &userId, const QString &role);
    bool changePassword(int userId, const QString &role, const QString &oldPass, const QString &newPass);

    QVector<Notice> getNotices();
    void addNotice(const QString &content, const QString &author);
    bool updateNotice(const QString &date, const QString &author, const QString &oldContent, const QString &newContent);
    bool deleteNotice(const QString &date, const QString &author, const QString &content);
    QString getDashboardStats(int userId, QString role);

    std::unique_ptr<Student> getStudent(int id);
    std::unique_ptr<Teacher> getTeacher(int id);
    QPair<QString, QString> getAdminProfile(int id);

    QVector<Task> getTasks(int userId);
    void addTask(int userId, const QString &description);
    void completeTask(int taskId, bool status);
    void deleteTask(int taskId);
    void deleteCompletedTasks(int userId);

    DailyPrayerStatus getDailyPrayers(int userId, QString date);
    void updateDailyPrayer(int userId, QString date, QString prayer, bool status);
    std::vector<std::unique_ptr<Habit>> getHabits(int userId);
    void addHabit(Habit *h);
    void updateHabit(Habit *h);
    void deleteHabit(int id);

    QVector<RoutineSession> getRoutineForDay(QString day, int semester = -1);
    void addRoutineItem(QString day, int serial, QString code, QString name, QString room, QString instructor, int semester);

    QVector<RoutineAdjustment> getRoutineAdjustments();
    void addRoutineAdjustment(const RoutineAdjustment &adj);
    QVector<RoutineSession> getEffectiveRoutine(QDate date, int semester = -1);
    QVector<RescheduleOption> getRescheduleOptions(QDate originDate, int originSerial, int semester, QString originCode, QString originRoom, QString instructorName);

    std::vector<std::unique_ptr<Course>> getTeacherCourses(int teacherId);
    std::unique_ptr<Course> getCourse(int id);
    QVector<Assessment> getAssessments();
    QVector<Assessment> getStudentAssessments(int studentId);
    void addAssessment(int courseId, QString title, QString type, QString date, int maxMarks);
    QVector<Assessment> getTeacherAssessments(int teacherId);
    std::vector<std::unique_ptr<Student>> getStudentsByEnrollment(int courseId);

    QVector<AttendanceRecord> getStudentAttendance(int studentId);
    std::vector<std::unique_ptr<Student>> getStudentsBySemester(int semester);

    double getGrade(int studentId, int assessmentId);
    void addGrade(int studentId, int assessmentId, double marks);
    QMap<int, double> getGradesForAssessment(int assessmentId);
    QSet<QString> getPresenceSet(int courseId);

    QVector<QString> getCourseDates(int courseId);
    bool isPresent(int courseId, int studentId, QString date);
    void markAttendance(int courseId, int studentId, QString date, bool present);

    QVector<Query> getQueries(int userId, QString role);
    void addQuery(int userId, int teacherId, QString question);
    void answerQuery(int queryId, QString answer);
    QVector<QPair<int, QString>> getTeacherList();

    QVector<AttendanceAnalytics> getLowAttendanceStudents(int courseId, double threshold = 75.0);
    double getOverallAttendancePercentage(int studentId);
    int generateAttendanceWarnings(int courseId, int teacherId, double threshold = 75.0);

    // Inbox and private correspondence operations
    QVector<PersonalMessage> getMessages(int userId);
    int getUnreadMessageCount(int userId);
    void sendMessage(int senderId, const QString &senderRole, int receiverId,
                     const QString &subject, const QString &content);
    void markMessageRead(int messageId);
    void deleteMessage(int messageId);
    int sendBulkGradeReports(int teacherId, int assessmentId, const QString &extraNote = "");

    // Property directory retrieval
    QVector<LostFoundPost> getLostFoundPosts();
    void addLostFoundPost(int posterId, const QString &posterName, const QString &posterRole,
                          const QString &type, const QString &itemName, const QString &description,
                          const QString &location);
    void claimLostFoundPost(int postId, const QString &claimerName);
    void deleteLostFoundPost(int postId);

    // Execution stack interaction
    void executeCommand(CommandPtr cmd);
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    QString undoDescription() const;
    QString redoDescription() const;

    // Data update subscriptions
    void addObserver(IDataObserver *observer);
    void removeObserver(IDataObserver *observer);

private:
    QVector<IDataObserver *> observers;
    void notifyObservers(DataType type);
    CommandHistory m_commandHistory;
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

/**
 * Wrapper component abstracting simple repetitive I/O queries
 */
template <typename T>
class CsvRepository
{
public:
    explicit CsvRepository(const QString &filename) : m_filename(filename) {}

    template <typename Func>
    std::unique_ptr<T> findById(int id, Func parser)
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

    template <typename Func, typename Filter>
    std::vector<std::unique_ptr<T>> findAll(Func parser, Filter filter)
    {
        std::vector<std::unique_ptr<T>> results;
        QVector<QStringList> data = CsvHandler::readCsv(m_filename);
        for (const auto &row : data)
        {
            if (filter(row))
            {
                if (auto obj = parser(row))
                    results.push_back(std::move(obj));
            }
        }
        return results;
    }

private:
    QString m_filename;
};
