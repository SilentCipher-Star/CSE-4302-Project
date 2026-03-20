#include "../include/manager_community.hpp"
#include "../include/manager_persons.hpp"
#include "../include/csvhandler.hpp"
#include <QDate>
#include <QDateTime>
#include <QMap>
#include <algorithm>

// === NOTICES ===
QVector<Notice> ManagerCommunity::getNotices()
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

    // Sort notices by date (newest first). Dates stored as yyyy-MM-dd.
    std::sort(notices.begin(), notices.end(), [](const Notice &a, const Notice &b)
              { return QDate::fromString(a.getDate(), Qt::ISODate) > QDate::fromString(b.getDate(), Qt::ISODate); });

    return notices;
}

void ManagerCommunity::addNotice(const QString &content, const QString &author)
{
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    CsvHandler::appendCsv("notices.csv", {date, author, content});
}

bool ManagerCommunity::updateNotice(const QString &date, const QString &author, const QString &oldContent, const QString &newContent)
{
    QVector<QStringList> data = CsvHandler::readCsv("notices.csv");
    for (QStringList &row : data)
    {
        if (row.size() >= 3 && row[0] == date && row[1] == author && row[2] == oldContent)
        {
            row[2] = newContent;
            CsvHandler::writeCsv("notices.csv", data);
            return true;
        }
    }
    return false;
}

bool ManagerCommunity::deleteNotice(const QString &date, const QString &author, const QString &content)
{
    QVector<QStringList> data = CsvHandler::readCsv("notices.csv");
    QVector<QStringList> kept;
    bool deleted = false;

    for (const QStringList &row : data)
    {
        if (!deleted && row.size() >= 3 && row[0] == date && row[1] == author && row[2] == content)
        {
            deleted = true;
            continue;
        }
        kept.append(row);
    }

    if (deleted)
        CsvHandler::writeCsv("notices.csv", kept);
    return deleted;
}

// === MESSAGES ===
QVector<PersonalMessage> ManagerCommunity::getMessages(int userId)
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
    std::sort(list.begin(), list.end(), [](const PersonalMessage &a, const PersonalMessage &b)
              { return a.getTimestamp() > b.getTimestamp(); });

    return list;
}

int ManagerCommunity::getUnreadCount(int userId)
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

void ManagerCommunity::sendMessage(int senderId, const QString &senderRole, int receiverId,
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
                                              "0" // unread
                                          });
}

void ManagerCommunity::markRead(int messageId)
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

void ManagerCommunity::deleteMessage(int messageId)
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

// === QUERIES ===
QVector<Query> ManagerCommunity::getQueries(int userId, QString role)
{
    QVector<Query> list;
    QVector<QStringList> data = CsvHandler::readCsv("queries.csv");

    // Pre-load names for display
    QMap<int, QString> studentNames;
    QVector<QStringList> sData = CsvHandler::readCsv("students.csv");
    for (const auto &row : sData)
        if (row.size() >= 2)
            studentNames[row[0].toInt()] = row[1];

    QMap<int, QString> teacherNames;
    QVector<QStringList> tData = CsvHandler::readCsv("teachers.csv");
    for (const auto &row : tData)
        if (row.size() >= 2)
            teacherNames[row[0].toInt()] = row[1];

    for (const auto &row : data)
    {
        if (row.size() >= 6)
        {
            int qId = row[0].toInt();
            int sId = row[1].toInt();
            int tId = row[2].toInt();

            bool isVisible = false;
            if (role == "Admin")
                isVisible = true;
            else if (role == "Student" && sId == userId)
                isVisible = true;
            else if (role == "Teacher" && tId == userId)
                isVisible = true;

            if (isVisible)
            {
                QString sName = studentNames.value(sId, "Unknown Student");
                QString tName = teacherNames.value(tId, "Unknown Teacher");
                list.append(Query(qId, sId, tId, sName, tName, row[3], row[4], row[5]));
            }
        }
    }
    return list;
}

void ManagerCommunity::addQuery(int userId, int teacherId, QString question)
{
    QVector<QStringList> data = CsvHandler::readCsv("queries.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (row.size() > 0)
            maxId = std::max(maxId, row[0].toInt());

    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
    CsvHandler::appendCsv("queries.csv", {QString::number(maxId + 1), QString::number(userId), QString::number(teacherId), question, "", ts});
}

void ManagerCommunity::answerQuery(int queryId, QString answer)
{
    QVector<QStringList> data = CsvHandler::readCsv("queries.csv");
    for (auto &row : data)
    {
        if (row.size() >= 6 && row[0].toInt() == queryId)
        {
            row[4] = answer;
        }
    }
    CsvHandler::writeCsv("queries.csv", data);
}

QVector<QPair<int, QString>> ManagerCommunity::getTeacherList()
{
    QVector<QPair<int, QString>> list;
    QVector<QStringList> data = CsvHandler::readCsv("teachers.csv");
    for (const auto &row : data)
    {
        if (row.size() >= 2)
            list.append({row[0].toInt(), row[1]});
    }
    return list;
}

// === LOST & FOUND ===
QVector<LostFoundPost> ManagerCommunity::getPosts()
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
    std::sort(list.begin(), list.end(), [](const LostFoundPost &a, const LostFoundPost &b)
              { return a.getDate() > b.getDate(); });

    return list;
}

void ManagerCommunity::addPost(int posterId, const QString &posterName, const QString &posterRole,
                               const QString &type, const QString &itemName, const QString &description,
                               const QString &location)
{
    QVector<QStringList> data = CsvHandler::readCsv("lostandfound.csv");
    int maxId = 0;
    for (const auto &row : data)
        if (!row.isEmpty())
            maxId = std::max(maxId, row[0].toInt());

    QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
    CsvHandler::appendCsv("lostandfound.csv", {QString::number(maxId + 1),
                                               QString::number(posterId),
                                               posterName,
                                               posterRole,
                                               type,
                                               itemName,
                                               description,
                                               location,
                                               ts,
                                               "OPEN",
                                               ""});
}

void ManagerCommunity::claimPost(int postId, const QString &claimerName)
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

void ManagerCommunity::deletePost(int postId)
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