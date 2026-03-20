#include "../include/appmanager.hpp"
#include "../include/manager_auth.hpp"
#include "../include/manager_persons.hpp"
#include "../include/manager_academics.hpp"
#include "../include/manager_productivity.hpp"
#include "../include/manager_community.hpp"
#include <QMap>

AcadenceManager::AcadenceManager() {}

// ============ Command Pattern - Undo/Redo ============
void AcadenceManager::executeCommand(CommandPtr cmd)
{
    cmd->execute();
    m_commandHistory.push(cmd);
    notifyObservers(cmd->affectedDataType());
}

void AcadenceManager::undo()
{
    if (!m_commandHistory.canUndo())
        return;
    CommandPtr cmd = m_commandHistory.undo();
    if (cmd)
        notifyObservers(cmd->affectedDataType());
}

void AcadenceManager::redo()
{
    if (!m_commandHistory.canRedo())
        return;
    CommandPtr cmd = m_commandHistory.redo();
    if (cmd)
        notifyObservers(cmd->affectedDataType());
}

bool AcadenceManager::canUndo() const { return m_commandHistory.canUndo(); }
bool AcadenceManager::canRedo() const { return m_commandHistory.canRedo(); }
QString AcadenceManager::undoDescription() const { return m_commandHistory.undoDescription(); }
QString AcadenceManager::redoDescription() const { return m_commandHistory.redoDescription(); }

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
        auto s = getStudent(userId);
        if (!s)
            return "GPA: N/A";
        int semester = s->getSemester();

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

        QMap<int, double> studentGrades;
        for (const auto &grow : gradeData)
        {
            if (grow.size() >= 3 && grow[0].toInt() == userId)
            {
                studentGrades[grow[1].toInt()] = grow[2].toDouble();
            }
        }

        for (const auto &arow : assData)
        {
            if (arow.size() < 6)
                continue;
            int aId = arow[0].toInt();
            int cId = arow[1].toInt();
            int maxMarks = arow[5].toInt();

            if (courseCredits.contains(cId))
            {
                if (studentGrades.contains(aId))
                {
                    courseMax[cId] += maxMarks;
                    courseObtained[cId] += studentGrades[aId];
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
        auto courses = getTeacherCourses(userId);
        int count = courses.size();
        return "Active Courses: " + QString::number(count);
    }
    return "System Active";
}

// ============ Notices ============
QVector<Notice> AcadenceManager::getNotices() { return ManagerCommunity::getNotices(); }
void AcadenceManager::addNotice(const QString &c, const QString &a)
{
    ManagerCommunity::addNotice(c, a);
    notifyObservers(DataType::Notices);
}
bool AcadenceManager::updateNotice(const QString &date, const QString &author, const QString &oldContent, const QString &newContent)
{
    const bool updated = ManagerCommunity::updateNotice(date, author, oldContent, newContent);
    if (updated)
        notifyObservers(DataType::Notices);
    return updated;
}
bool AcadenceManager::deleteNotice(const QString &date, const QString &author, const QString &content)
{
    const bool deleted = ManagerCommunity::deleteNotice(date, author, content);
    if (deleted)
        notifyObservers(DataType::Notices);
    return deleted;
}

// ============ Persons ============
std::unique_ptr<Student> AcadenceManager::getStudent(int id) { return ManagerPersons::getStudent(id); }
std::unique_ptr<Teacher> AcadenceManager::getTeacher(int id) { return ManagerPersons::getTeacher(id); }
QPair<QString, QString> AcadenceManager::getAdminProfile(int id) { return ManagerPersons::getAdminProfile(id); }

// ============ Tasks ============
QVector<Task> AcadenceManager::getTasks(int userId) { return ManagerProductivity::getTasks(userId); }
void AcadenceManager::addTask(int userId, const QString &desc)
{
    ManagerProductivity::addTask(userId, desc);
    notifyObservers(DataType::Tasks);
}
void AcadenceManager::completeTask(int taskId, bool status)
{
    ManagerProductivity::completeTask(taskId, status);
    notifyObservers(DataType::Tasks);
}
void AcadenceManager::deleteTask(int taskId)
{
    ManagerProductivity::deleteTask(taskId);
    notifyObservers(DataType::Tasks);
}
void AcadenceManager::deleteCompletedTasks(int userId)
{
    ManagerProductivity::deleteCompletedTasks(userId);
    notifyObservers(DataType::Tasks);
}

// ============ Prayers ============
DailyPrayerStatus AcadenceManager::getDailyPrayers(int userId, QString date) { return ManagerProductivity::getDailyPrayers(userId, date); }
void AcadenceManager::updateDailyPrayer(int userId, QString date, QString prayer, bool status)
{
    ManagerProductivity::updateDailyPrayer(userId, date, prayer, status);
    notifyObservers(DataType::Habits);
}

// ============ Habits ============
std::vector<std::unique_ptr<Habit>> AcadenceManager::getHabits(int userId) { return ManagerProductivity::getHabits(userId); }
void AcadenceManager::addHabit(Habit *h)
{
    ManagerProductivity::addHabit(h);
    notifyObservers(DataType::Habits);
}
void AcadenceManager::updateHabit(Habit *h)
{
    ManagerProductivity::updateHabit(h);
    notifyObservers(DataType::Habits);
}
void AcadenceManager::deleteHabit(int id)
{
    ManagerProductivity::deleteHabit(id);
    notifyObservers(DataType::Habits);
}

// ============ Routine ============
QVector<RoutineSession> AcadenceManager::getRoutineForDay(QString day, int semester) { return ManagerProductivity::getRoutineForDay(day, semester); }
void AcadenceManager::addRoutineItem(QString day, int serial, QString code, QString name, QString room, QString instructor, int semester)
{
    ManagerProductivity::addRoutineItem(day, serial, code, name, room, instructor, semester);
    notifyObservers(DataType::Routine);
}
QVector<RoutineAdjustment> AcadenceManager::getRoutineAdjustments() { return ManagerProductivity::getRoutineAdjustments(); }
void AcadenceManager::addRoutineAdjustment(const RoutineAdjustment &adj)
{
    ManagerProductivity::addRoutineAdjustment(adj);
    notifyObservers(DataType::Routine);
}
QVector<RoutineSession> AcadenceManager::getEffectiveRoutine(QDate date, int semester) { return ManagerProductivity::getEffectiveRoutine(date, semester); }
QVector<RescheduleOption> AcadenceManager::getRescheduleOptions(QDate originDate, int originSerial, int semester, QString originCode, QString originRoom, QString instructorName)
{
    return ManagerProductivity::getRescheduleOptions(originDate, originSerial, semester, originCode, originRoom, instructorName);
}

// ============ Academics ============
std::vector<std::unique_ptr<Course>> AcadenceManager::getTeacherCourses(int teacherId) { return ManagerAcademics::getTeacherCourses(teacherId); }
std::unique_ptr<Course> AcadenceManager::getCourse(int id) { return ManagerAcademics::getCourse(id); }
QVector<Assessment> AcadenceManager::getAssessments() { return ManagerAcademics::getAssessments(); }
QVector<Assessment> AcadenceManager::getTeacherAssessments(int teacherId) { return ManagerAcademics::getTeacherAssessments(teacherId); }
QVector<Assessment> AcadenceManager::getStudentAssessments(int studentId) { return ManagerAcademics::getStudentAssessments(studentId); }
void AcadenceManager::addAssessment(int courseId, QString title, QString type, QString date, int maxMarks)
{
    ManagerAcademics::addAssessment(courseId, title, type, date, maxMarks);
    notifyObservers(DataType::Academics);
}
std::vector<std::unique_ptr<Student>> AcadenceManager::getStudentsByEnrollment(int courseId) { return ManagerAcademics::getStudentsByEnrollment(courseId); }
std::vector<std::unique_ptr<Student>> AcadenceManager::getStudentsBySemester(int semester) { return ManagerAcademics::getStudentsBySemester(semester); }
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
QMap<int, double> AcadenceManager::getGradesForAssessment(int assessmentId) { return ManagerAcademics::getGradesForAssessment(assessmentId); }
QSet<QString> AcadenceManager::getPresenceSet(int courseId) { return ManagerAcademics::getPresenceSet(courseId); }

// ============ Queries ============
QVector<Query> AcadenceManager::getQueries(int userId, QString role) { return ManagerCommunity::getQueries(userId, role); }
void AcadenceManager::addQuery(int userId, int teacherId, QString question)
{
    ManagerCommunity::addQuery(userId, teacherId, question);
    notifyObservers(DataType::Queries);
}
void AcadenceManager::answerQuery(int queryId, QString answer)
{
    ManagerCommunity::answerQuery(queryId, answer);
    notifyObservers(DataType::Queries);
}
QVector<QPair<int, QString>> AcadenceManager::getTeacherList() { return ManagerCommunity::getTeacherList(); }

// ============ Personal Messages ============
QVector<PersonalMessage> AcadenceManager::getMessages(int userId) { return ManagerCommunity::getMessages(userId); }
int AcadenceManager::getUnreadMessageCount(int userId) { return ManagerCommunity::getUnreadCount(userId); }
void AcadenceManager::sendMessage(int senderId, const QString &senderRole, int receiverId,
                                  const QString &subject, const QString &content)
{
    ManagerCommunity::sendMessage(senderId, senderRole, receiverId, subject, content);
    notifyObservers(DataType::Messages);
}
void AcadenceManager::markMessageRead(int messageId)
{
    ManagerCommunity::markRead(messageId);
    notifyObservers(DataType::Messages);
}
void AcadenceManager::deleteMessage(int messageId)
{
    ManagerCommunity::deleteMessage(messageId);
    notifyObservers(DataType::Messages);
}

// ============ Lost & Found ============
QVector<LostFoundPost> AcadenceManager::getLostFoundPosts() { return ManagerCommunity::getPosts(); }
void AcadenceManager::addLostFoundPost(int posterId, const QString &posterName, const QString &posterRole,
                                       const QString &type, const QString &itemName, const QString &description,
                                       const QString &location)
{
    ManagerCommunity::addPost(posterId, posterName, posterRole, type, itemName, description, location);
    notifyObservers(DataType::LostFound);
}
void AcadenceManager::claimLostFoundPost(int postId, const QString &claimerName)
{
    ManagerCommunity::claimPost(postId, claimerName);
    notifyObservers(DataType::LostFound);
}
void AcadenceManager::deleteLostFoundPost(int postId)
{
    ManagerCommunity::deletePost(postId);
    notifyObservers(DataType::LostFound);
}
