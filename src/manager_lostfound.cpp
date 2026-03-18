#include "../include/manager_lostfound.hpp"
#include "../include/csvhandler.hpp"
#include <QDateTime>
#include <algorithm>

QVector<LostFoundPost> ManagerLostFound::getPosts()
{
    QVector<LostFoundPost> list;
    QVector<QStringList> data = CsvHandler::readCsv("lostandfound.csv");

    for (const auto &row : data)
    {
        // Format: id,posterId,posterName,posterRole,type,itemName,description,location,date,status,claimedBy
        if (row.size() >= 10)
        {
            list.append(LostFoundPost(
                row[0].toInt(),
                row[1].toInt(),
                row[2],
                row[3],
                row[4],
                row[5],
                row[6],
                row[7],
                row[8],
                row[9],
                row.size() >= 11 ? row[10] : ""));
        }
    }

    // Sort newest first
    std::sort(list.begin(), list.end(), [](const LostFoundPost &a, const LostFoundPost &b) {
        return a.getDate() > b.getDate();
    });

    return list;
}

void ManagerLostFound::addPost(int posterId, const QString &posterName, const QString &posterRole,
                               const QString &type, const QString &itemName, const QString &description,
                               const QString &location)
{
    QVector<QStringList> data = CsvHandler::readCsv("lostandfound.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (!row.isEmpty())
            maxId = std::max(maxId, row[0].toInt());

    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
    CsvHandler::appendCsv("lostandfound.csv", {
        QString::number(maxId + 1),
        QString::number(posterId),
        posterName,
        posterRole,
        type,
        itemName,
        description,
        location,
        ts,
        "OPEN",
        ""
    });
}

void ManagerLostFound::claimPost(int postId, const QString &claimerName)
{
    QVector<QStringList> data = CsvHandler::readCsv("lostandfound.csv");
    for (auto &row : data)
    {
        if (row.size() >= 10 && row[0].toInt() == postId)
        {
            row[9] = "CLAIMED";
            if (row.size() >= 11)
                row[10] = claimerName;
            else
                row.append(claimerName);
        }
    }
    CsvHandler::writeCsv("lostandfound.csv", data);
}

void ManagerLostFound::deletePost(int postId)
{
    QVector<QStringList> data = CsvHandler::readCsv("lostandfound.csv");
    QVector<QStringList> kept;
    for (const auto &row : data)
    {
        if (row.size() >= 1 && row[0].toInt() == postId)
            continue;
        kept.append(row);
    }
    CsvHandler::writeCsv("lostandfound.csv", kept);
}
