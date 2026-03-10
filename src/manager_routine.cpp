#include "../include/manager_routine.hpp"
#include "../include/csvhandler.hpp"
#include <QTime>
#include <algorithm>

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

QVector<RoutineSession> ManagerRoutine::getRoutineForDay(QString day, int semester)
{
    QVector<QStringList> data = CsvHandler::readCsv("routine.csv");
    return parseRoutineSessions(data, day, semester);
}

void ManagerRoutine::addRoutineItem(QString day, int serial, QString code, QString name, QString room, QString instructor, int semester)
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

QVector<RoutineAdjustment> ManagerRoutine::getRoutineAdjustments()
{
    QVector<QStringList> data = CsvHandler::readCsv("routine_adjustments.csv");
    return parseAdjustments(data);
}

void ManagerRoutine::addRoutineAdjustment(const RoutineAdjustment &adj)
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

QVector<RoutineSession> ManagerRoutine::getEffectiveRoutine(QDate date, int semester)
{
    QString dayName = date.toString("dddd");
    QVector<RoutineSession> baseRoutine = getRoutineForDay(dayName, semester);
    QVector<RoutineAdjustment> adjustments = getRoutineAdjustments();
    return computeEffectiveRoutine(date, semester, baseRoutine, adjustments);
}

QVector<RescheduleOption> ManagerRoutine::getRescheduleOptions(QDate originDate, int originSerial, int semester, QString originCode, QString originRoom, QString instructorName)
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
