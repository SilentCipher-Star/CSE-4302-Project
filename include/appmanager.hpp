#ifndef ACADENCEMANAGER_HPP
#define ACADENCEMANAGER_HPP

#include <QString>
#include <QVector>
#include <QDate>
#include <QDebug>
#include "student.hpp"
#include "teacher.hpp"
#include "course.hpp"
#include "habit.hpp"
#include "routine.hpp"
#include "exceptions.hpp"
#include "csvhandler.hpp"
#include "command_history.hpp"

class Notice
{
private:
    QString date;
    QString author;
    QString content;

public:
    Notice() = default;
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

    friend QDebug operator<<(QDebug dbg, const Assessment &a)
    {
        dbg.nospace() << "Assessment(id=" << a.id
                      << ", title=" << a.title
                      << ", course=" << a.courseName
                      << ", date=" << a.date
                      << ", maxMarks=" << a.maxMarks << ")";
        return dbg.space();
    }
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

struct AttendanceAnalytics
{
    int studentId;
    QString studentName;
    int courseId;
    QString courseName;
    int totalClasses;
    int attendedClasses;
    double percentage;
};

class Query
{
private:
    int id;
    int studentId;
    int teacherId;
    QString studentName;
    QString teacherName;
    QString question;
    QString answer;
    QString timestamp;

public:
    Query(int id, int sid, int tid, QString sname, QString tname, QString q, QString a, QString ts)
        : id(id), studentId(sid), teacherId(tid), studentName(sname), teacherName(tname), question(q), answer(a), timestamp(ts) {}

    int getId() const { return id; }
    int getStudentId() const { return studentId; }
    int getTeacherId() const { return teacherId; }
    QString getStudentName() const { return studentName; }
    QString getTeacherName() const { return teacherName; }
    QString getQuestion() const { return question; }
    QString getAnswer() const { return answer; }
    QString getTimestamp() const { return timestamp; }
};

class PersonalMessage
{
private:
    int id;
    int senderId;
    QString senderRole;
    QString senderName;
    int receiverId;
    QString subject;
    QString content;
    QString timestamp;
    bool isRead;

public:
    PersonalMessage(int id, int sid, QString srole, QString sname, int rid,
                    QString subj, QString cont, QString ts, bool read)
        : id(id), senderId(sid), senderRole(srole), senderName(sname),
          receiverId(rid), subject(subj), content(cont), timestamp(ts), isRead(read) {}

    int getId() const { return id; }
    int getSenderId() const { return senderId; }
    QString getSenderRole() const { return senderRole; }
    QString getSenderName() const { return senderName; }
    int getReceiverId() const { return receiverId; }
    QString getSubject() const { return subject; }
    QString getContent() const { return content; }
    QString getTimestamp() const { return timestamp; }
    bool getIsRead() const { return isRead; }
};

class LostFoundPost
{
private:
    int id;
    int posterId;
    QString posterName;
    QString posterRole;
    QString type;        // "LOST" or "FOUND"
    QString itemName;
    QString description;
    QString location;
    QString date;
    QString status;      // "OPEN" or "CLAIMED"
    QString claimedBy;

public:
    LostFoundPost(int id, int posterId, QString posterName, QString posterRole,
                  QString type, QString itemName, QString description,
                  QString location, QString date, QString status, QString claimedBy)
        : id(id), posterId(posterId), posterName(posterName), posterRole(posterRole),
          type(type), itemName(itemName), description(description),
          location(location), date(date), status(status), claimedBy(claimedBy) {}

    int getId() const { return id; }
    int getPosterId() const { return posterId; }
    QString getPosterName() const { return posterName; }
    QString getPosterRole() const { return posterRole; }
    QString getType() const { return type; }
    QString getItemName() const { return itemName; }
    QString getDescription() const { return description; }
    QString getLocation() const { return location; }
    QString getDate() const { return date; }
    QString getStatus() const { return status; }
    QString getClaimedBy() const { return claimedBy; }
};

struct RescheduleOption
{
    QString displayText;
    RoutineAdjustment adjustment;
    RoutineAdjustment secondaryAdjustment;
};

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

    // Add delegates - these call the manager modules and notify observers
    QString login(const QString &username, const QString &password, int &userId, const QString &role);
    bool changePassword(int userId, const QString &role, const QString &oldPass, const QString &newPass);

    QVector<Notice> getNotices();
    void addNotice(const QString &content, const QString &author);
    bool updateNotice(const QString &date, const QString &author, const QString &oldContent, const QString &newContent);
    bool deleteNotice(const QString &date, const QString &author, const QString &content);
    QString getDashboardStats(int userId, QString role);

    Student *getStudent(int id);
    Teacher *getTeacher(int id);
    QPair<QString, QString> getAdminProfile(int id);

    QVector<Task> getTasks(int userId);
    void addTask(int userId, const QString &description);
    void completeTask(int taskId, bool status);
    void deleteTask(int taskId);
    void deleteCompletedTasks(int userId);

    DailyPrayerStatus getDailyPrayers(int userId, QString date);
    void updateDailyPrayer(int userId, QString date, QString prayer, bool status);
    QVector<Habit *> getHabits(int userId);
    void addHabit(Habit *h);
    void updateHabit(Habit *h);
    void deleteHabit(int id);

    QVector<RoutineSession> getRoutineForDay(QString day, int semester = -1);
    void addRoutineItem(QString day, int serial, QString code, QString name, QString room, QString instructor, int semester);

    QVector<RoutineAdjustment> getRoutineAdjustments();
    void addRoutineAdjustment(const RoutineAdjustment &adj);
    QVector<RoutineSession> getEffectiveRoutine(QDate date, int semester = -1);
    QVector<RescheduleOption> getRescheduleOptions(QDate originDate, int originSerial, int semester, QString originCode, QString originRoom, QString instructorName);

    QVector<Course *> getTeacherCourses(int teacherId);
    Course *getCourse(int id);
    QVector<Assessment> getAssessments();
    QVector<Assessment> getStudentAssessments(int studentId);
    void addAssessment(int courseId, QString title, QString type, QString date, int maxMarks);
    QVector<Assessment> getTeacherAssessments(int teacherId);
    QVector<Student *> getStudentsByEnrollment(int courseId);

    QVector<AttendanceRecord> getStudentAttendance(int studentId);
    QVector<Student *> getStudentsBySemester(int semester);

    double getGrade(int studentId, int assessmentId);
    void addGrade(int studentId, int assessmentId, double marks);

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

    // Personal Messages
    QVector<PersonalMessage> getMessages(int userId);
    int getUnreadMessageCount(int userId);
    void sendMessage(int senderId, const QString &senderRole, int receiverId,
                     const QString &subject, const QString &content);
    void markMessageRead(int messageId);
    void deleteMessage(int messageId);
    int sendBulkGradeReports(int teacherId, int assessmentId, const QString &extraNote = "");

    // Lost & Found
    QVector<LostFoundPost> getLostFoundPosts();
    void addLostFoundPost(int posterId, const QString &posterName, const QString &posterRole,
                          const QString &type, const QString &itemName, const QString &description,
                          const QString &location);
    void claimLostFoundPost(int postId, const QString &claimerName);
    void deleteLostFoundPost(int postId);

    // Command Pattern - Undo/Redo support
    void executeCommand(CommandPtr cmd);
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    QString undoDescription() const;
    QString redoDescription() const;

    // Observer pattern
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

// Small CSV repository helper template for single-file lookups
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

#endif // ACADENCEMANAGER_HPP