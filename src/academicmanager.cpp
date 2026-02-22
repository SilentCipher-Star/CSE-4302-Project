#include "../include/academicmanager.hpp"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMap>
#include <QDir>
#include <QCoreApplication>

AcadenceManager::AcadenceManager()
{
}

QString AcadenceManager::login(const QString &username, const QString &password, int &userId)
{
    QVector<QStringList> admins = CsvHandler::readCsv("admins.csv");
    for (const auto &row : admins)
    {
        if (row.size() >= 3 && row[1] == username && row[2] == password)
        {
            userId = row[0].toInt();
            return "Admin";
        }
    }

    QVector<QStringList> students = CsvHandler::readCsv("students.csv");
    for (const auto &row : students)
    {
        if (row.size() >= 5 && row[3] == username && row[4] == password)
        {
            userId = row[0].toInt();
            return "Student";
        }
    }

    QVector<QStringList> teachers = CsvHandler::readCsv("teachers.csv");
    for (const auto &row : teachers)
    {
        if (row.size() >= 5 && row[3] == username && row[4] == password)
        {
            userId = row[0].toInt();
            return "Teacher";
        }
    }
    return "";
}

bool AcadenceManager::changePassword(int userId, const QString &role, const QString &oldPass, const QString &newPass)
{
    QString filename;
    int passIndex = 4;

    if (role == "Student")
    {
        filename = "students.csv";
    }
    else if (role == "Teacher")
    {
        filename = "teachers.csv";
    }
    else if (role == "Admin")
    {
        filename = "admins.csv";
        passIndex = 2;
    }
    else
    {
        return false;
    }

    QVector<QStringList> data = CsvHandler::readCsv(filename);
    bool found = false;

    for (auto &row : data)
    {
        if (row.size() > passIndex && row[0].toInt() == userId)
        {
            if (row[passIndex] == oldPass)
            {
                row[passIndex] = newPass;
                found = true;
            }
            else
            {
                throw Acadence::Exception("Old password does not match.");
            }
            break;
        }
    }

    if (found)
    {
        CsvHandler::writeCsv(filename, data);
        return true;
    }
    throw Acadence::Exception("User not found.");
}

QVector<Notice> AcadenceManager::getNotices()
{
    QVector<Notice> notices;
    QVector<QStringList> data = CsvHandler::readCsv("notices.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 3)
        {
            notices.append(Notice(row[0], row[1], row[2]));
        }
    }
    return notices;
}

void AcadenceManager::addNotice(const QString &content, const QString &author)
{
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    CsvHandler::appendCsv("notices.csv", {date, author, content});
}

QString AcadenceManager::getNextClass(int userId)
{
    Student *s = getStudent(userId);
    if (!s)
        return "No Data";

    int semester = s->getSemester();
    delete s;

    QString day = QDate::currentDate().toString("dddd");
    QVector<RoutineSession> routine = getRoutineForDay(day, semester);
    QTime now = QTime::currentTime();

    for (const auto &item : routine)
    {
        QTime start = QTime::fromString(item.getStartTime(), "HH:mm");
        if (start > now)
        {
            return item.getCourseCode() + " (" + item.getStartTime() + ")";
        }
    }
    return "No more classes";
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
            {
                courseCredits[row[0].toInt()] = row[5].toInt();
            }
        }

        QMap<int, double> courseObtained;
        QMap<int, double> courseMax;

        QVector<QStringList> assData = CsvHandler::readCsv("assessments.csv");
        QVector<QStringList> gradeData = CsvHandler::readCsv("grades.csv");

        // Calculate obtained marks for each course
        for (const auto &arow : assData)
        {
            if (arow.size() < 6)
                continue;
            int aId = arow[0].toInt();
            int cId = arow[1].toInt();
            int maxMarks = arow[5].toInt();

            if (courseCredits.contains(cId))
            {
                bool isGraded = false;
                double obtained = 0.0;
                for (const auto &grow : gradeData)
                {
                    if (grow.size() >= 3 && grow[0].toInt() == userId && grow[1].toInt() == aId)
                    {
                        obtained = grow[2].toDouble();
                        isGraded = true;
                        break;
                    }
                }

                if (isGraded)
                {
                    courseMax[cId] += maxMarks;
                    courseObtained[cId] += obtained;
                }
            }
        }

        double totalPoints = 0;
        double totalCredits = 0;

        for (auto it = courseCredits.begin(); it != courseCredits.end(); ++it)
        {
            int cId = it.key();
            int credits = it.value();
            double max = courseMax.value(cId, 0);

            // Calculate Grade Point based on percentage
            if (max > 0)
            {
                double pct = (courseObtained.value(cId, 0) / max) * 100.0;
                double gp = 0.0;
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

                totalPoints += (gp * credits);
                totalCredits += credits;
            }
        }

        if (totalCredits > 0)
            return "GPA: " + QString::number(totalPoints / totalCredits, 'f', 2);

        return "GPA: N/A";
    }
    else if (role == "Teacher")
    {
        return "Active Courses: " + QString::number(getTeacherCourses(userId).size());
    }
    return "System Active";
}

Student *AcadenceManager::getStudent(int id)
{
    QVector<QStringList> data = CsvHandler::readCsv("students.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 8 && row[0].toInt() == id)
        {
            Student *s = new Student(row[0].toInt(), row[1], row[2], row[5], row[6], row[7].toInt());
            s->setUsername(row[3]);
            s->setPassword(row[4]);

            if (row.size() >= 9)
                s->setDateAdmission(QDate::fromString(row[8], Qt::ISODate));
            if (row.size() >= 10)
                s->setGpa(row[9].toDouble());

            return s;
        }
    }
    return nullptr;
}

Teacher *AcadenceManager::getTeacher(int id)
{
    QVector<QStringList> data = CsvHandler::readCsv("teachers.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 7 && row[0].toInt() == id)
        {
            Teacher *t = new Teacher(row[0].toInt(), row[1], row[2], row[5], row[6]);
            t->setUsername(row[3]);
            t->setPassword(row[4]);

            if (row.size() >= 8)
                t->setSalary(row[7].toDouble());

            return t;
        }
    }
    return nullptr;
}

QVector<Task> AcadenceManager::getTasks(int userId)
{
    QVector<Task> tasks;
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 4 && row[1].toInt() == userId)
        {
            tasks.append(Task(row[0].toInt(), row[2], (row[3] == "1")));
        }
    }
    return tasks;
}

void AcadenceManager::addTask(int userId, const QString &description)
{
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());

    CsvHandler::appendCsv("tasks.csv", {QString::number(maxId + 1), QString::number(userId), description, "0"});
}

void AcadenceManager::completeTask(int taskId, bool status)
{
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    for (auto &row : data)
    {
        if (row.size() >= 4 && row[0].toInt() == taskId)
        {
            row[3] = status ? "1" : "0";
        }
    }
    CsvHandler::writeCsv("tasks.csv", data);
}

DailyPrayerStatus AcadenceManager::getDailyPrayers(int userId, QString date)
{
    QVector<QStringList> data = CsvHandler::readCsv("prayers.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 7 && row[0].toInt() == userId && row[1] == date)
        {
            return DailyPrayerStatus(row[2] == "1", row[3] == "1", row[4] == "1", row[5] == "1", row[6] == "1");
        }
    }
    return DailyPrayerStatus(false, false, false, false, false);
}

void AcadenceManager::updateDailyPrayer(int userId, QString date, QString prayer, bool status)
{
    QVector<QStringList> data = CsvHandler::readCsv("prayers.csv");
    bool found = false;

    int prayerIdx = -1;
    if (prayer == "fajr")
        prayerIdx = 2;
    else if (prayer == "dhuhr")
        prayerIdx = 3;
    else if (prayer == "asr")
        prayerIdx = 4;
    else if (prayer == "maghrib")
        prayerIdx = 5;
    else if (prayer == "isha")
        prayerIdx = 6;

    for (auto &row : data)
    {
        if (row.size() >= 7 && row[0].toInt() == userId && row[1] == date)
        {
            if (prayerIdx != -1)
                row[prayerIdx] = status ? "1" : "0";
            found = true;
        }
    }

    if (!found)
    {
        QStringList newRow = {QString::number(userId), date, "0", "0", "0", "0", "0"};
        if (prayerIdx != -1)
            newRow[prayerIdx] = status ? "1" : "0";
        data.append(newRow);
    }
    CsvHandler::writeCsv("prayers.csv", data);
}

QVector<Habit *> AcadenceManager::getHabits(int userId)
{
    QVector<Habit *> habits;
    QVector<QStringList> data = CsvHandler::readCsv("habits.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 11 && row[1].toInt() == userId)
        {
            int id = row[0].toInt();
            QString name = row[2];
            HabitType type = (row[3] == "Duration") ? HabitType::DURATION : HabitType::COUNT;
            Frequency freq = (row[4] == "Daily") ? Frequency::DAILY : Frequency::WEEKLY;
            int target = row[5].toInt();
            int current = row[6].toInt();
            int streak = row[7].toInt();
            QDate lastDate = QDate::fromString(row[8], Qt::ISODate);
            bool isComp = (row[9] == "1");
            QString unit = row[10];

            Habit *h = nullptr;
            if (type == HabitType::DURATION)
            {
                auto *dh = new DurationHabit(id, userId, name, freq, target);
                dh->currentMinutes = current;
                h = dh;
            }
            else
            {
                auto *ch = new CountHabit(id, userId, name, freq, target, unit);
                ch->currentCount = current;
                h = ch;
            }

            h->streak = streak;
            h->lastUpdated = lastDate;
            h->isCompleted = isComp;

            if (h->checkReset())
            {
                updateHabit(h);
            }
            habits.append(h);
        }
    }
    return habits;
}

void AcadenceManager::addHabit(Habit *h)
{
    QVector<QStringList> data = CsvHandler::readCsv("habits.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());
    h->id = maxId + 1;

    QString typeStr = (h->type == HabitType::DURATION) ? "Duration" : "Count";
    QString freqStr = (h->frequency == Frequency::DAILY) ? "Daily" : "Weekly";
    QString unit = "";
    int target = 0;

    if (auto *dh = dynamic_cast<DurationHabit *>(h))
        target = dh->targetMinutes;
    else if (auto *ch = dynamic_cast<CountHabit *>(h))
    {
        target = ch->targetCount;
        unit = ch->unit;
    }

    CsvHandler::appendCsv("habits.csv", {QString::number(h->id), QString::number(h->studentId), h->name, typeStr, freqStr,
                                         QString::number(target), "0", "0", QDate::currentDate().toString(Qt::ISODate), "0", unit});
}

void AcadenceManager::updateHabit(Habit *h)
{
    QVector<QStringList> data = CsvHandler::readCsv("habits.csv");
    for (auto &row : data)
    {
        if (row.size() >= 11 && row[0].toInt() == h->id)
        {
            int current = 0;
            if (auto *dh = dynamic_cast<DurationHabit *>(h))
                current = dh->currentMinutes;
            else if (auto *ch = dynamic_cast<CountHabit *>(h))
                current = ch->currentCount;

            row[6] = QString::number(current);
            row[7] = QString::number(h->streak);
            row[8] = h->lastUpdated.toString(Qt::ISODate);
            row[9] = h->isCompleted ? "1" : "0";
        }
    }
    CsvHandler::writeCsv("habits.csv", data);
}

void AcadenceManager::deleteHabit(int id)
{
    QVector<QStringList> data = CsvHandler::readCsv("habits.csv");
    QVector<QStringList> newData;
    for (const auto &row : data)
    {
        if (row.size() > 0 && row[0].toInt() != id)
        {
            newData.append(row);
        }
    }
    CsvHandler::writeCsv("habits.csv", newData);
}

QVector<RoutineSession> AcadenceManager::getRoutineForDay(QString day, int semester)
{
    WeeklyRoutine weeklyRoutine;
    QVector<QStringList> data = CsvHandler::readCsv("routine.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 8)
        {
            if (semester == -1 || row[7].toInt() == semester)
            {
                weeklyRoutine.addSession(RoutineSession(row[0], row[1], row[2], row[3], row[4], row[5], row[6], row[7].toInt()));
            }
        }
    }
    return weeklyRoutine.getSessionsForDay(day);
}

void AcadenceManager::addRoutineItem(QString day, QString start, QString end, QString code, QString name, QString room, QString instructor, int semester)
{
    CsvHandler::appendCsv("routine.csv", {day, start, end, code, name, room, instructor, QString::number(semester)});
}

QVector<Course *> AcadenceManager::getTeacherCourses(int teacherId)
{
    QVector<Course *> courses;
    QVector<QStringList> data = CsvHandler::readCsv("courses.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 6 && row[3].toInt() == teacherId)
        {
            courses.append(new Course(row[0].toInt(), row[1], row[2], row[3].toInt(), row[4].toInt(), row[5].toInt()));
        }
    }
    return courses;
}

Course *AcadenceManager::getCourse(int id)
{
    QVector<QStringList> data = CsvHandler::readCsv("courses.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 6 && row[0].toInt() == id)
        {
            return new Course(row[0].toInt(), row[1], row[2], row[3].toInt(), row[4].toInt(), row[5].toInt());
        }
    }
    return nullptr;
}

QVector<Assessment> AcadenceManager::getAssessments()
{
    QVector<Assessment> list;
    QVector<QStringList> assessmentData = CsvHandler::readCsv("assessments.csv");

    // Optimization: Read courses once to create a lookup map
    // This avoids opening the courses file for every single assessment row.
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

void AcadenceManager::addAssessment(int courseId, QString title, QString type, QString date, int maxMarks)
{
    QVector<QStringList> data = CsvHandler::readCsv("assessments.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());

    CsvHandler::appendCsv("assessments.csv", {QString::number(maxId + 1), QString::number(courseId), title, type, date, QString::number(maxMarks)});
}

QVector<AttendanceRecord> AcadenceManager::getStudentAttendance(int studentId)
{
    QVector<AttendanceRecord> records;
    Student *s = getStudent(studentId);
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
            if (row.size() >= 4 && row[0].toInt() == cid)
            {
                uniqueDates.insert(row[2]);
                if (row[1].toInt() == studentId && row[3] == "1")
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

QVector<Student *> AcadenceManager::getStudentsBySemester(int semester)
{
    QVector<Student *> list;
    QVector<QStringList> data = CsvHandler::readCsv("students.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 6 && row[5].toInt() == semester)
        {
            list.append(new Student(row[0].toInt(), row[1], row[2], row[3], row[4], row[5].toInt()));
        }
    }
    return list;
}

double AcadenceManager::getGrade(int studentId, int assessmentId)
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

void AcadenceManager::addGrade(int studentId, int assessmentId, double marks)
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

QVector<QString> AcadenceManager::getCourseDates(int courseId)
{
    QSet<QString> dates;
    QVector<QStringList> data = CsvHandler::readCsv("attendance.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 4 && row[0].toInt() == courseId)
        {
            dates.insert(row[2]);
        }
    }
    QVector<QString> list = dates.values();
    std::sort(list.begin(), list.end());
    return list;
}

bool AcadenceManager::isPresent(int courseId, int studentId, QString date)
{
    QVector<QStringList> data = CsvHandler::readCsv("attendance.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 4 && row[0].toInt() == courseId && row[1].toInt() == studentId && row[2] == date)
        {
            return row[3] == "1";
        }
    }
    return false;
}

void AcadenceManager::markAttendance(int courseId, int studentId, QString date, bool present)
{
    QVector<QStringList> data = CsvHandler::readCsv("attendance.csv");
    bool found = false;
    for (auto &row : data)
    {
        if (row.size() >= 4 && row[0].toInt() == courseId && row[1].toInt() == studentId && row[2] == date)
        {
            row[3] = present ? "1" : "0";
            found = true;
        }
    }
    if (!found)
    {
        data.append({QString::number(courseId), QString::number(studentId), date, present ? "1" : "0"});
    }
    CsvHandler::writeCsv("attendance.csv", data);
}

QVector<Query> AcadenceManager::getQueries(int userId, QString role)
{
    QVector<Query> list;
    QVector<QStringList> data = CsvHandler::readCsv("queries.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 4)
        {
            if (role == "Teacher" || role == "Admin" || row[1].toInt() == userId)
            {
                QString sName = "Student";
                Student *s = getStudent(row[1].toInt());
                if (s)
                {
                    sName = s->getName();
                    delete s;
                }

                list.append(Query(row[0].toInt(), row[1].toInt(), sName, row[2], row[3]));
            }
        }
    }
    return list;
}

void AcadenceManager::addQuery(int userId, QString question)
{
    QVector<QStringList> data = CsvHandler::readCsv("queries.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());

    CsvHandler::appendCsv("queries.csv", {QString::number(maxId + 1), QString::number(userId), question, ""});
}

void AcadenceManager::answerQuery(int queryId, QString answer)
{
    QVector<QStringList> data = CsvHandler::readCsv("queries.csv");
    QVector<QString> lines;
    for (auto &row : data)
    {
        if (row.size() >= 4 && row[0].toInt() == queryId)
        {
            row[3] = answer;
        }
    }
    CsvHandler::writeCsv("queries.csv", data);
}