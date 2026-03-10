#ifndef MANAGER_NOTICES_HPP
#define MANAGER_NOTICES_HPP

#include <QString>
#include <QVector>
#include "appmanager.hpp"

class ManagerNotices
{
public:
    static QVector<Notice> getNotices();
    static void addNotice(const QString &content, const QString &author);
    static bool updateNotice(const QString &date, const QString &author, const QString &oldContent, const QString &newContent);
    static bool deleteNotice(const QString &date, const QString &author, const QString &content);
};

#endif // MANAGER_NOTICES_HPP
