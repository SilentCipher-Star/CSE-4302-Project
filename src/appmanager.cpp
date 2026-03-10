#include "../include/appmanager.hpp"
#include "../include/manager_auth.hpp"
#include "../include/manager_notices.hpp"
#include "../include/manager_persons.hpp"
#include "../include/manager_prayers.hpp"
#include "../include/manager_routine.hpp"
#include "../include/manager_academics.hpp"
#include "../include/manager_queries.hpp"
#include "../include/manager_tasks.hpp"
#include "../include/manager_habits.hpp"
#include "../include/manager_messages.hpp"
#include <QMap>

AcadenceManager::AcadenceManager() {}

void AcadenceManager::addObserver(IDataObserver *observer)
{
    if (!observers.contains(observer))
        observers.append(observer);
}

void AcadenceManager::removeObserver(IDataObserver *observer)
{
    observers.removeAll(observer);
}

void AcadenceManager::notifyObservers(DataType type)
{
    for (auto *obs : observers)
        obs->onDataChanged(type);
}

// ============ Authentication ============
QString AcadenceManager::login(const QString &username, const QString &password, int &userId, const QString &role)
{
    return ManagerAuth::login(username, password, userId, role);
}

bool AcadenceManager::changePassword(int userId, const QString &role, const QString &oldPass, const QString &newPass)
{
    bool result = ManagerAuth::changePassword(userId, role, oldPass, newPass);
    if (result)
        notifyObservers(DataType::Profile);
    return result;
}

QString AcadenceManager::getDashboardStats(int userId, QString role)
{
    if (role == "Student")
    {
        Student *s = getStudent(userId);
        if (!s)
            return "GPA: N/A";
        int semester = s->getSemester();
        delete s;

        QMap<int, int> courseCredits;
        QVector<QStringList> courseData = CsvHandler::readCsv("courses.csv");
        for (const auto &row : courseData)
        {
            if (row.size() >= 6 && row[4].toInt() == semester)
                courseCredits[row[0].toInt()] = row[5].toInt();
        }

        QMap<int, double> courseObtained;
        QMap<int, double> courseMax;
        QVector<QStringList> assData = CsvHandler::readCsv("assessments.csv");
        QVector<QStringList> gradeData = CsvHandler::readCsv("grades.csv");

        for (const auto &arow : assData)
        {
            if (arow.size() < 6)
                continue;
            int aId = arow[0].toInt();
            int cId = arow[1].toInt();
            int maxMarks = arow[5].toInt();

            if (courseCredits.contains(cId))
            {
                for (const auto &grow : gradeData)
                {
                    if (grow.size() >= 3 && grow[0].toInt() == userId && grow[1].toInt() == aId)
                    {
                        courseMax[cId] += maxMarks;
                        courseObtained[cId] += grow[2].toDouble();
                        break;
                    }
                }
            }
        }

        double totalPoints = 0, totalCredits = 0;
        for (auto it = courseCredits.begin(); it != courseCredits.end(); ++it)
        {
            int cId = it.key();
            double max = courseMax.value(cId, 0);
            if (max > 0)
            {
                double pct = (courseObtained.value(cId, 0) / max) * 100.0;
                double gp = 0;
                if (pct >= 80)
                    gp = 4.00;
                else if (pct >= 75)
                    gp = 3.75;
                else if (pct >= 70)
                    gp = 3.50;
                else if (pct >= 65)
                    gp = 3.25;
                else if (pct >= 60)
                    gp = 3.00;
                else if (pct >= 55)
                    gp = 2.75;
                else if (pct >= 50)
                    gp = 2.50;
                else if (pct >= 45)
                    gp = 2.25;
                else if (pct >= 40)
                    gp = 2.00;
                totalPoints += (gp * it.value());
                totalCredits += it.value();
            }
        }
        if (totalCredits > 0)
            return "GPA: " + QString::number(totalPoints / totalCredits, 'f', 2);
        return "GPA: N/A";
    }
    else if (role == "Teacher")
    {
        QVector<Course *> courses = getTeacherCourses(userId);
        int count = courses.size();
        qDeleteAll(courses);
        return "Active Courses: " + QString::number(count);
    }
    return "System Active";
}

// ============ Notices ============
QVector<Notice> AcadenceManager::getNotices() { return ManagerNotices::getNotices(); }
void AcadenceManager::addNotice(const QString &c, const QString &a)
{
    ManagerNotices::addNotice(c, a);
    notifyObservers(DataType::Notices);
}
bool AcadenceManager::updateNotice(const QString &date, const QString &author, const QString &oldContent, const QString &newContent)
{
    const bool updated = ManagerNotices::updateNotice(date, author, oldContent, newContent);
    if (updated)
        notifyObservers(DataType::Notices);
    return updated;
}
bool AcadenceManager::deleteNotice(const QString &date, const QString &author, const QString &content)
{
    const bool deleted = ManagerNotices::deleteNotice(date, author, content);
    if (deleted)
        notifyObservers(DataType::Notices);
    return deleted;
}

// ============ Persons ============
Student *AcadenceManager::getStudent(int id) { return ManagerPersons::getStudent(id); }
Teacher *AcadenceManager::getTeacher(int id) { return ManagerPersons::getTeacher(id); }
QPair<QString, QString> AcadenceManager::getAdminProfile(int id) { return ManagerPersons::getAdminProfile(id); }

// ============ Tasks ============
QVector<Task> AcadenceManager::getTasks(int userId) { return ManagerTasks::getTasks(userId); }
void AcadenceManager::addTask(int userId, const QString &desc)
{
    ManagerTasks::addTask(userId, desc);
    notifyObservers(DataType::Tasks);
}
void AcadenceManager::completeTask(int taskId, bool status)
{
    ManagerTasks::completeTask(taskId, status);
    notifyObservers(DataType::Tasks);
}
void AcadenceManager::deleteTask(int taskId)
{
    ManagerTasks::deleteTask(taskId);
    notifyObservers(DataType::Tasks);
}
void AcadenceManager::deleteCompletedTasks(int userId)
{
    ManagerTasks::deleteCompletedTasks(userId);
    notifyObservers(DataType::Tasks);
}

// ============ Prayers ============
DailyPrayerStatus AcadenceManager::getDailyPrayers(int userId, QString date) { return ManagerPrayers::getDailyPrayers(userId, date); }
void AcadenceManager::updateDailyPrayer(int userId, QString date, QString prayer, bool status)
{
    ManagerPrayers::updateDailyPrayer(userId, date, prayer, status);
    notifyObservers(DataType::Habits);
}

// ============ Habits ============
QVector<Habit *> AcadenceManager::getHabits(int userId) { return ManagerHabits::getHabits(userId); }
void AcadenceManager::addHabit(Habit *h)
{
    ManagerHabits::addHabit(h);
    notifyObservers(DataType::Habits);
}
void AcadenceManager::updateHabit(Habit *h)
{
    ManagerHabits::updateHabit(h);
    notifyObservers(DataType::Habits);
}
void AcadenceManager::deleteHabit(int id)
{
    ManagerHabits::deleteHabit(id);
    notifyObservers(DataType::Habits);
}

// ============ Routine ============
QVector<RoutineSession> AcadenceManager::getRoutineForDay(QString day, int semester) { return ManagerRoutine::getRoutineForDay(day, semester); }
void AcadenceManager::addRoutineItem(QString day, int serial, QString code, QString name, QString room, QString instructor, int semester)
{
    ManagerRoutine::addRoutineItem(day, serial, code, name, room, instructor, semester);
    notifyObservers(DataType::Routine);
}
QVector<RoutineAdjustment> AcadenceManager::getRoutineAdjustments() { return ManagerRoutine::getRoutineAdjustments(); }
void AcadenceManager::addRoutineAdjustment(const RoutineAdjustment &adj)
{
    ManagerRoutine::addRoutineAdjustment(adj);
    notifyObservers(DataType::Routine);
}
QVector<RoutineSession> AcadenceManager::getEffectiveRoutine(QDate date, int semester) { return ManagerRoutine::getEffectiveRoutine(date, semester); }
QVector<RescheduleOption> AcadenceManager::getRescheduleOptions(QDate originDate, int originSerial, int semester, QString originCode, QString originRoom, QString instructorName)
{
    return ManagerRoutine::getRescheduleOptions(originDate, originSerial, semester, originCode, originRoom, instructorName);
}

// ============ Academics ============
QVector<Course *> AcadenceManager::getTeacherCourses(int teacherId) { return ManagerAcademics::getTeacherCourses(teacherId); }
Course *AcadenceManager::getCourse(int id) { return ManagerAcademics::getCourse(id); }
QVector<Assessment> AcadenceManager::getAssessments() { return ManagerAcademics::getAssessments(); }
QVector<Assessment> AcadenceManager::getTeacherAssessments(int teacherId) { return ManagerAcademics::getTeacherAssessments(teacherId); }
QVector<Assessment> AcadenceManager::getStudentAssessments(int studentId) { return ManagerAcademics::getStudentAssessments(studentId); }
void AcadenceManager::addAssessment(int courseId, QString title, QString type, QString date, int maxMarks)
{
    ManagerAcademics::addAssessment(courseId, title, type, date, maxMarks);
    notifyObservers(DataType::Academics);
}
QVector<Student *> AcadenceManager::getStudentsByEnrollment(int courseId) { return ManagerAcademics::getStudentsByEnrollment(courseId); }
QVector<Student *> AcadenceManager::getStudentsBySemester(int semester) { return ManagerAcademics::getStudentsBySemester(semester); }
QVector<AttendanceRecord> AcadenceManager::getStudentAttendance(int studentId) { return ManagerAcademics::getStudentAttendance(studentId); }
QVector<QString> AcadenceManager::getCourseDates(int courseId) { return ManagerAcademics::getCourseDates(courseId); }
bool AcadenceManager::isPresent(int courseId, int studentId, QString date) { return ManagerAcademics::isPresent(courseId, studentId, date); }
void AcadenceManager::markAttendance(int courseId, int studentId, QString date, bool present)
{
    ManagerAcademics::markAttendance(courseId, studentId, date, present);
    notifyObservers(DataType::Academics);
}
double AcadenceManager::getGrade(int studentId, int assessmentId) { return ManagerAcademics::getGrade(studentId, assessmentId); }
void AcadenceManager::addGrade(int studentId, int assessmentId, double marks)
{
    ManagerAcademics::addGrade(studentId, assessmentId, marks);
    notifyObservers(DataType::Academics);
}

// ============ Queries ============
QVector<Query> AcadenceManager::getQueries(int userId, QString role) { return ManagerQueries::getQueries(userId, role); }
void AcadenceManager::addQuery(int userId, int teacherId, QString question)
{
    ManagerQueries::addQuery(userId, teacherId, question);
    notifyObservers(DataType::Queries);
}
void AcadenceManager::answerQuery(int queryId, QString answer)
{
    ManagerQueries::answerQuery(queryId, answer);
    notifyObservers(DataType::Queries);
}
QVector<QPair<int, QString>> AcadenceManager::getTeacherList() { return ManagerQueries::getTeacherList(); }

// ============ Personal Messages ============
QVector<PersonalMessage> AcadenceManager::getMessages(int userId) { return ManagerMessages::getMessages(userId); }
int AcadenceManager::getUnreadMessageCount(int userId) { return ManagerMessages::getUnreadCount(userId); }
void AcadenceManager::sendMessage(int senderId, const QString &senderRole, int receiverId,
                                  const QString &subject, const QString &content)
{
    ManagerMessages::sendMessage(senderId, senderRole, receiverId, subject, content);
    notifyObservers(DataType::Messages);
}
void AcadenceManager::markMessageRead(int messageId)
{
    ManagerMessages::markRead(messageId);
    notifyObservers(DataType::Messages);
}
void AcadenceManager::deleteMessage(int messageId)
{
    ManagerMessages::deleteMessage(messageId);
    notifyObservers(DataType::Messages);
}
