#ifndef CSVHANDLER_HPP
#define CSVHANDLER_HPP

#include <QString>
#include <QVector>
#include <QStringList>

class CsvHandler
{
public:
    static QString getDataDirectory();
    static QVector<QStringList> readCsv(const QString &filename);
    static void writeCsv(const QString &filename, const QVector<QStringList> &data);
    static void appendCsv(const QString &filename, const QStringList &fields);
};

#endif // CSVHANDLER_HPP