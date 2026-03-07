#ifndef MANAGER_QUERIES_HPP
#define MANAGER_QUERIES_HPP

#include <QString>
#include <QVector>
#include <QPair>
#include "appmanager.hpp"

class ManagerQueries
{
public:
    static QVector<Query> getQueries(int userId, QString role);
    static void addQuery(int userId, int teacherId, QString question);
    static void answerQuery(int queryId, QString answer);
    static QVector<QPair<int, QString>> getTeacherList();
};

#endif // MANAGER_QUERIES_HPP
