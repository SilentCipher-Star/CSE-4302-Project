#include "../include/manager_messages.hpp"
#include "../include/manager_persons.hpp"
#include "../include/csvhandler.hpp"
#include <QDateTime>
#include <QMap>
#include <algorithm>

QVector<PersonalMessage> ManagerMessages::getMessages(int userId)
{
    QVector<PersonalMessage> list;
    QVector<QStringList> data = CsvHandler::readCsv("messages.csv");

    // Pre-load names
    QMap<int, QString> studentNames;
    QVector<QStringList> sData = CsvHandler::readCsv("students.csv");
    for (const auto &row : sData)
        if (row.size() >= 2)
            studentNames[row[0].toInt()] = row[1].trimmed();

    QMap<int, QString> teacherNames;
    QVector<QStringList> tData = CsvHandler::readCsv("teachers.csv");
    for (const auto &row : tData)
        if (row.size() >= 2)
            teacherNames[row[0].toInt()] = row[1].trimmed();

    for (const auto &row : data)
    {
        // Format: id, senderId, senderRole, receiverId, subject, content, timestamp, isRead
        if (row.size() >= 8 && row[3].toInt() == userId)
        {
            int sid = row[1].toInt();
            QString sRole = row[2];
            QString sName;
            if (sRole == "Teacher")
                sName = teacherNames.value(sid, "Unknown Teacher");
            else if (sRole == "Student")
                sName = studentNames.value(sid, "Unknown Student");
            else
                sName = "Admin";

            list.append(PersonalMessage(
                row[0].toInt(), sid, sRole, sName, row[3].toInt(),
                row[4], row[5], row[6], row[7].toInt() == 1));
        }
    }

    // Sort newest first
    std::sort(list.begin(), list.end(), [](const PersonalMessage &a, const PersonalMessage &b) {
        return a.getTimestamp() > b.getTimestamp();
    });

    return list;
}

int ManagerMessages::getUnreadCount(int userId)
{
    int count = 0;
    QVector<QStringList> data = CsvHandler::readCsv("messages.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 8 && row[3].toInt() == userId && row[7] == "0")
            count++;
    }
    return count;
}

void ManagerMessages::sendMessage(int senderId, const QString &senderRole, int receiverId,
                                  const QString &subject, const QString &content)
{
    QVector<QStringList> data = CsvHandler::readCsv("messages.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (!row.isEmpty())
            maxId = std::max(maxId, row[0].toInt());

    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
    CsvHandler::appendCsv("messages.csv", {
        QString::number(maxId + 1),
        QString::number(senderId),
        senderRole,
        QString::number(receiverId),
        subject,
        content,
        ts,
        "0"  // unread
    });
}

void ManagerMessages::markRead(int messageId)
{
    QVector<QStringList> data = CsvHandler::readCsv("messages.csv");
    for (auto &row : data)
    {
        if (row.size() >= 8 && row[0].toInt() == messageId)
        {
            row[7] = "1";
        }
    }
    CsvHandler::writeCsv("messages.csv", data);
}

void ManagerMessages::deleteMessage(int messageId)
{
    QVector<QStringList> data = CsvHandler::readCsv("messages.csv");
    QVector<QStringList> kept;
    for (const auto &row : data)
    {
        if (row.size() >= 8 && row[0].toInt() == messageId)
            continue;
        kept.append(row);
    }
    CsvHandler::writeCsv("messages.csv", kept);
}
