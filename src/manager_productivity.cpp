#include "../include/manager_productivity.hpp"
#include "../include/csvhandler.hpp"
#include <QDate>
#include <QTime>
#include <algorithm>

// --- HABITS ---
std::vector<std::unique_ptr<Habit>> ManagerProductivity::getHabits(int userId)
{
    std::vector<std::unique_ptr<Habit>> habits;
    QVector<QStringList> data = CsvHandler::readCsv("habits.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 11 && row[1].toInt() == userId)
        {
            int id = row[0].toInt();
            QString name = row[2];
            HabitType type = HabitType::COUNT;
            if (row[3] == "Duration")
                type = HabitType::DURATION;
            else if (row[3] == "Workout")
                type = HabitType::WORKOUT;

            Frequency freq = (row[4] == "Daily") ? Frequency::DAILY : Frequency::WEEKLY;
            QString targetStr = row[5], currentStr = row[6];
            int streak = row[7].toInt();
            QDate lastDate = QDate::fromString(row[8], Qt::ISODate);
            bool isComp = (row[9] == "1");
            QString unit = row[10];

            std::unique_ptr<Habit> h;
            if (type == HabitType::DURATION)
            {
                auto dh = std::make_unique<DurationHabit>(id, userId, name, freq, targetStr.toInt());
                dh->setCurrentMinutes(currentStr.toDouble());
                h = std::move(dh);
            }
            else if (type == HabitType::COUNT)
            {
                auto ch = std::make_unique<CountHabit>(id, userId, name, freq, targetStr.toInt(), unit);
                ch->setCurrentCount(currentStr.toInt());
                h = std::move(ch);
            }
            else if (type == HabitType::WORKOUT)
            {
                QStringList targets = targetStr.split('|');
                int tMin = (targets.size() > 0) ? targets[0].toInt() : 0;
                int tCnt = (targets.size() > 1) ? targets[1].toInt() : 0;
                auto wh = std::make_unique<WorkoutHabit>(id, userId, name, freq, tMin, tCnt, unit);
                wh->deserializeValue(currentStr);
                h = std::move(wh);
            }

            if (h)
            {
                h->setStreak(streak);
                h->setLastUpdated(lastDate);
                h->setIsCompleted(isComp);
                if (h->checkReset())
                    updateHabit(h.get());
                habits.push_back(std::move(h));
            }
        }
    }
    return habits;
}

void ManagerProductivity::addHabit(Habit *h)
{
    QVector<QStringList> data = CsvHandler::readCsv("habits.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());
    h->setId(maxId + 1);

    const QString typeStr = h->getTypeString();
    const QString freqStr = h->getFrequencyString();
    const QString currentStr = h->serializeValue();
    const auto *chPtr = dynamic_cast<const CountHabit *>(h);
    const QString unit = chPtr ? chPtr->getUnit() : "";

    QString targetStr;
    if (auto *wh = dynamic_cast<WorkoutHabit *>(h))
    {
        targetStr = QString("%1|%2").arg(wh->getTargetMinutes()).arg(wh->getTargetCount());
    }
    else if (auto *dh = dynamic_cast<DurationHabit *>(h))
    {
        targetStr = QString::number(dh->getTargetMinutes());
    }
    else if (auto *ch = dynamic_cast<CountHabit *>(h))
    {
        targetStr = QString::number(ch->getTargetCount());
    }

    CsvHandler::appendCsv("habits.csv", {QString::number(h->getId()), QString::number(h->getStudentId()), h->getName(), typeStr, freqStr,
                                         targetStr, currentStr, "0", QDate::currentDate().toString(Qt::ISODate), "0", unit});
}

void ManagerProductivity::updateHabit(Habit *h)
{
    QVector<QStringList> data = CsvHandler::readCsv("habits.csv");
    for (auto &row : data)
    {
        if (row.size() >= 11 && row[0].toInt() == h->getId())
        {
            row[6] = h->serializeValue();
            row[7] = QString::number(h->getStreak());
            row[8] = h->getLastUpdated().toString(Qt::ISODate);
            row[9] = h->getIsCompleted() ? "1" : "0";
        }
    }
    CsvHandler::writeCsv("habits.csv", data);
}

void ManagerProductivity::deleteHabit(int id)
{
    QVector<QStringList> data = CsvHandler::readCsv("habits.csv");
    QVector<QStringList> newData;
    for (const auto &row : data)
    {
        if (row.size() > 0 && row[0].toInt() != id)
            newData.append(row);
    }
    CsvHandler::writeCsv("habits.csv", newData);
}

// --- TASKS ---
QVector<Task> ManagerProductivity::getTasks(int userId)
{
    QVector<Task> tasks;
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 4 && row[1].toInt() == userId)
            tasks.append(Task(row[0].toInt(), row[2], (row[3] == "1")));
    }
    return tasks;
}

void ManagerProductivity::addTask(int userId, const QString &description)
{
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());
    CsvHandler::appendCsv("tasks.csv", {QString::number(maxId + 1), QString::number(userId), description, "0"});
}

void ManagerProductivity::completeTask(int taskId, bool status)
{
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    for (auto &row : data)
    {
        if (row.size() >= 4 && row[0].toInt() == taskId)
            row[3] = status ? "1" : "0";
    }
    CsvHandler::writeCsv("tasks.csv", data);
}

void ManagerProductivity::deleteTask(int taskId)
{
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    QVector<QStringList> newData;
    for (const auto &row : data)
    {
        if (row.size() > 0 && row[0].toInt() != taskId)
            newData.append(row);
    }
    CsvHandler::writeCsv("tasks.csv", newData);
}

void ManagerProductivity::deleteCompletedTasks(int userId)
{
    QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
    QVector<QStringList> newData;
    for (const auto &row : data)
    {
        if (row.size() < 4 || row[1].toInt() != userId || row[3] != "1")
            newData.append(row);
    }
    CsvHandler::writeCsv("tasks.csv", newData);
}

// --- PRAYERS ---
DailyPrayerStatus ManagerProductivity::getDailyPrayers(int userId, QString date)
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

void ManagerProductivity::updateDailyPrayer(int userId, QString date, QString prayer, bool status)
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

// --- ROUTINE ---
static QVector<RoutineSession> parseRoutineSessions(const QVector<QStringList> &data, QString day, int semester)
{
    WeeklyRoutine weeklyRoutine;
    for (const auto &row : data)
    {
        if (row.size() >= 7)
        {
            if (semester == -1 || row[6].toInt() == semester)
            {
                int serial = row[1].toInt();
                QString start, end;
                if (serial == 1)
                {
                    start = "09:00";
                    end = "10:00";
                }
                else if (serial == 2)
                {
                    start = "10:00";
                    end = "11:00";
                }
                else if (serial == 3)
                {
                    start = "11:00";
                    end = "12:00";
                }
                else if (serial == 4)
                {
                    start = "12:00";
                    end = "13:00";
                }
                else if (serial == 5)
                {
                    start = "14:00";
                    end = "15:00";
                }
                else
                {
                    start = "00:00";
                    end = "00:00";
                }

                weeklyRoutine.addSession(RoutineSession(row[0], start, end, row[2], row[3], row[4], row[5], row[6].toInt()));
            }
        }
    }
    return weeklyRoutine.getSessionsForDay(day);
}

QVector<RoutineSession> ManagerProductivity::getRoutineForDay(QString day, int semester)
{
    QVector<QStringList> data = CsvHandler::readCsv("routine.csv");
    return parseRoutineSessions(data, day, semester);
}

void ManagerProductivity::addRoutineItem(QString day, int serial, QString code, QString name, QString room, QString instructor, int semester)
{
    CsvHandler::appendCsv("routine.csv", {day, QString::number(serial), code, name, room, instructor, QString::number(semester)});
}

static QVector<RoutineAdjustment> parseAdjustments(const QVector<QStringList> &data)
{
    QVector<RoutineAdjustment> list;
    for (const auto &row : data)
    {
        if (row.size() >= 10)
        {
            RoutineAdjustment adj;
            adj.originalDate = row[0];
            adj.originalSerial = row[1].toInt();
            adj.type = row[2];
            adj.newDate = row[3];
            adj.newSerial = row[4].toInt();
            adj.courseCode = row[5];
            adj.courseName = row[6];
            adj.room = row[7];
            adj.instructor = row[8];
            adj.semester = row[9].toInt();
            list.append(adj);
        }
    }
    return list;
}

QVector<RoutineAdjustment> ManagerProductivity::getRoutineAdjustments()
{
    QVector<QStringList> data = CsvHandler::readCsv("routine_adjustments.csv");
    return parseAdjustments(data);
}

void ManagerProductivity::addRoutineAdjustment(const RoutineAdjustment &adj)
{
    CsvHandler::appendCsv("routine_adjustments.csv", {adj.originalDate, QString::number(adj.originalSerial), adj.type,
                                                      adj.newDate, QString::number(adj.newSerial),
                                                      adj.courseCode, adj.courseName, adj.room, adj.instructor,
                                                      QString::number(adj.semester)});
}

static QVector<RoutineSession> computeEffectiveRoutine(QDate date, int semester, const QVector<RoutineSession> &baseRoutine, const QVector<RoutineAdjustment> &adjustments)
{
    QString dayName = date.toString("dddd");
    QVector<RoutineSession> effectiveRoutine;

    // 1. Process base routine: exclude if cancelled or moved FROM this date
    for (const auto &session : baseRoutine)
    {
        bool modified = false;
        for (const auto &adj : adjustments)
        {
            QString start;
            if (adj.originalSerial == 1)
                start = "09:00";
            else if (adj.originalSerial == 2)
                start = "10:00";
            else if (adj.originalSerial == 3)
                start = "11:00";
            else if (adj.originalSerial == 4)
                start = "12:00";
            else if (adj.originalSerial == 5)
                start = "14:00";

            if (adj.originalDate == date.toString(Qt::ISODate) &&
                start == session.getStartTime() &&
                adj.courseCode == session.getCourseCode())
            {
                modified = true; // It's either cancelled or moved away
                break;
            }
        }
        if (!modified)
        {
            effectiveRoutine.append(session);
        }
    }

    // 2. Add sessions moved TO this date
    for (const auto &adj : adjustments)
    {
        if (adj.type == "RESCHEDULE" && adj.newDate == date.toString(Qt::ISODate))
        {
            // Check if this rescheduled slot has been cancelled
            bool isCancelled = false;
            for (const auto &cancelAdj : adjustments)
            {
                if (cancelAdj.type == "CANCEL" &&
                    cancelAdj.originalDate == adj.newDate &&
                    cancelAdj.originalSerial == adj.newSerial &&
                    cancelAdj.courseCode == adj.courseCode)
                {
                    isCancelled = true;
                    break;
                }
            }
            if (isCancelled)
                continue;

            if (semester == -1 || adj.semester == semester)
            {
                // Calculate end time (assuming 1 hour duration for simplicity if not stored)
                QString start, end;
                if (adj.newSerial == 1)
                {
                    start = "09:00";
                    end = "10:00";
                }
                else if (adj.newSerial == 2)
                {
                    start = "10:00";
                    end = "11:00";
                }
                else if (adj.newSerial == 3)
                {
                    start = "11:00";
                    end = "12:00";
                }
                else if (adj.newSerial == 4)
                {
                    start = "12:00";
                    end = "13:00";
                }
                else if (adj.newSerial == 5)
                {
                    start = "14:00";
                    end = "15:00";
                }
                else
                {
                    start = "00:00";
                    end = "00:00";
                }

                effectiveRoutine.append(RoutineSession(
                    dayName, start, end,
                    adj.courseCode, adj.courseName, adj.room, adj.instructor, adj.semester));
            }
        }
    }

    // Sort by start time
    std::sort(effectiveRoutine.begin(), effectiveRoutine.end(), [](const RoutineSession &a, const RoutineSession &b)
              { return QTime::fromString(a.getStartTime(), "HH:mm") < QTime::fromString(b.getStartTime(), "HH:mm"); });

    return effectiveRoutine;
}

QVector<RoutineSession> ManagerProductivity::getEffectiveRoutine(QDate date, int semester)
{
    QString dayName = date.toString("dddd");
    QVector<RoutineSession> baseRoutine = getRoutineForDay(dayName, semester);
    QVector<RoutineAdjustment> adjustments = getRoutineAdjustments();
    return computeEffectiveRoutine(date, semester, baseRoutine, adjustments);
}

QVector<RescheduleOption> ManagerProductivity::getRescheduleOptions(QDate originDate, int originSerial, int semester, QString originCode, QString originRoom, QString instructorName)
{
    QVector<RescheduleOption> options;
    QDate d = QDate::currentDate();

    // Optimization: Read CSVs once
    QVector<QStringList> rawRoutine = CsvHandler::readCsv("routine.csv");
    QVector<QStringList> rawAdjustments = CsvHandler::readCsv("routine_adjustments.csv");
    QVector<RoutineAdjustment> adjustments = parseAdjustments(rawAdjustments);

    for (int i = 0; i < 14; ++i)
    {
        QDate targetDate = d.addDays(i);
        if (targetDate.dayOfWeek() > 5)
            continue; // Skip weekends

        QString dayName = targetDate.toString("dddd");
        QVector<RoutineSession> baseRoutine = parseRoutineSessions(rawRoutine, dayName, semester);
        QVector<RoutineSession> daily = computeEffectiveRoutine(targetDate, semester, baseRoutine, adjustments);

        // Check standard slots: 09:00, 10:00, 11:00, 12:00, 14:00
        QList<int> serials = {1, 2, 3, 4, 5};
        for (int s : serials)
        {
            QString slotTime;
            if (s == 1)
                slotTime = "09:00";
            else if (s == 2)
                slotTime = "10:00";
            else if (s == 3)
                slotTime = "11:00";
            else if (s == 4)
                slotTime = "12:00";
            else if (s == 5)
                slotTime = "14:00";

            if (targetDate == originDate && s == originSerial)
                continue;

            bool occupied = false;
            RoutineSession occupiedSession("", "", "", "", "", "", "", 0);

            for (const auto &sess : daily)
            {
                if (sess.getStartTime() == slotTime)
                {
                    occupied = true;
                    occupiedSession = sess;
                    break;
                }
            }

            RoutineAdjustment adj;
            adj.originalDate = originDate.toString(Qt::ISODate);
            adj.originalSerial = originSerial;
            adj.type = "RESCHEDULE";
            adj.newDate = targetDate.toString(Qt::ISODate);
            adj.newSerial = s;
            adj.courseCode = originCode;
            adj.courseName = "Rescheduled"; // Simplified, or fetch name if needed
            adj.room = originRoom;
            adj.instructor = instructorName;
            adj.semester = semester;

            RescheduleOption opt;
            opt.adjustment = adj;

            if (!occupied)
            {
                opt.displayText = targetDate.toString("ddd MMM dd") + " at " + slotTime + " (Empty Slot)";
                options.append(opt);
            }
            else
            {
                // Exchange option
                opt.displayText = targetDate.toString("ddd MMM dd") + " at " + slotTime + " (Exchange with " + occupiedSession.getCourseCode() + ")";

                RoutineAdjustment adj2;
                adj2.originalDate = targetDate.toString(Qt::ISODate);
                adj2.originalSerial = s;
                adj2.type = "RESCHEDULE";
                adj2.newDate = originDate.toString(Qt::ISODate);
                adj2.newSerial = originSerial;
                adj2.courseCode = occupiedSession.getCourseCode();
                adj2.courseName = occupiedSession.getCourseName();
                adj2.room = occupiedSession.getRoom();
                adj2.instructor = occupiedSession.getInstructor();
                adj2.semester = occupiedSession.getSemester();

                opt.secondaryAdjustment = adj2;
                options.append(opt);
            }
        }
    }
    return options;
}