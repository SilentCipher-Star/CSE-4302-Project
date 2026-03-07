#include "../include/manager_habits.hpp"
#include "../include/csvhandler.hpp"
#include <QDate>
#include <algorithm>

QVector<Habit *> ManagerHabits::getHabits(int userId)
{
    QVector<Habit *> habits;
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

            Habit *h = nullptr;
            if (type == HabitType::DURATION)
            {
                auto *dh = new DurationHabit(id, userId, name, freq, targetStr.toInt());
                dh->currentMinutes = currentStr.toDouble();
                h = dh;
            }
            else if (type == HabitType::COUNT)
            {
                auto *ch = new CountHabit(id, userId, name, freq, targetStr.toInt(), unit);
                ch->currentCount = currentStr.toInt();
                h = ch;
            }
            else if (type == HabitType::WORKOUT)
            {
                QStringList targets = targetStr.split('|');
                int tMin = (targets.size() > 0) ? targets[0].toInt() : 0;
                int tCnt = (targets.size() > 1) ? targets[1].toInt() : 0;
                auto *wh = new WorkoutHabit(id, userId, name, freq, tMin, tCnt, unit);
                wh->deserializeValue(currentStr);
                h = wh;
            }

            h->streak = streak;
            h->lastUpdated = lastDate;
            h->isCompleted = isComp;
            if (h->checkReset())
                updateHabit(h);
            habits.append(h);
        }
    }
    return habits;
}

void ManagerHabits::addHabit(Habit *h)
{
    QVector<QStringList> data = CsvHandler::readCsv("habits.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());
    h->id = maxId + 1;

    QString typeStr = h->getTypeString();
    QString freqStr = (h->frequency == Frequency::DAILY) ? "Daily" : "Weekly";
    QString unit = "", targetStr = "0", currentStr = "0";

    if (auto *wh = dynamic_cast<WorkoutHabit *>(h))
    {
        targetStr = QString("%1|%2").arg(wh->targetMinutes).arg(wh->targetCount);
        currentStr = wh->serializeValue();
        unit = wh->unit;
    }
    else if (auto *dh = dynamic_cast<DurationHabit *>(h))
    {
        targetStr = QString::number(dh->targetMinutes);
        currentStr = QString::number(dh->currentMinutes, 'f', 4);
    }
    else if (auto *ch = dynamic_cast<CountHabit *>(h))
    {
        targetStr = QString::number(ch->targetCount);
        currentStr = QString::number(ch->currentCount);
        unit = ch->unit;
    }

    CsvHandler::appendCsv("habits.csv", {QString::number(h->id), QString::number(h->studentId), h->name, typeStr, freqStr,
                                         targetStr, currentStr, "0", QDate::currentDate().toString(Qt::ISODate), "0", unit});
}

void ManagerHabits::updateHabit(Habit *h)
{
    QVector<QStringList> data = CsvHandler::readCsv("habits.csv");
    for (auto &row : data)
    {
        if (row.size() >= 11 && row[0].toInt() == h->id)
        {
            QString currentStr = "0";
            if (auto *wh = dynamic_cast<WorkoutHabit *>(h))
                currentStr = wh->serializeValue();
            else if (auto *dh = dynamic_cast<DurationHabit *>(h))
                currentStr = QString::number(dh->currentMinutes, 'f', 4);
            else if (auto *ch = dynamic_cast<CountHabit *>(h))
                currentStr = QString::number(ch->currentCount);

            row[6] = currentStr;
            row[7] = QString::number(h->streak);
            row[8] = h->lastUpdated.toString(Qt::ISODate);
            row[9] = h->isCompleted ? "1" : "0";
        }
    }
    CsvHandler::writeCsv("habits.csv", data);
}

void ManagerHabits::deleteHabit(int id)
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
