#include "../include/manager_prayers.hpp"
#include "../include/csvhandler.hpp"

DailyPrayerStatus ManagerPrayers::getDailyPrayers(int userId, QString date)
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

void ManagerPrayers::updateDailyPrayer(int userId, QString date, QString prayer, bool status)
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
