#include "../include/manager_notices.hpp"
#include <QDate>
#include <algorithm>

QVector<Notice> ManagerNotices::getNotices()
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

void ManagerNotices::addNotice(const QString &content, const QString &author)
{
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    CsvHandler::appendCsv("notices.csv", {date, author, content});
}

bool ManagerNotices::updateNotice(const QString &date, const QString &author, const QString &oldContent, const QString &newContent)
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

bool ManagerNotices::deleteNotice(const QString &date, const QString &author, const QString &content)
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
