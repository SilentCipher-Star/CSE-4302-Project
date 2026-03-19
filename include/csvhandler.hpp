#pragma once
#include <QString>
#include <QVector>
#include <QStringList>
#include <QMap>

class CsvHandler
{
public:
    static QString getDataDirectory();
    static QVector<QStringList> readCsv(const QString &filename);
    static void writeCsv(const QString &filename, const QVector<QStringList> &data);
    static void appendCsv(const QString &filename, const QStringList &fields);
    static void initialize();
    static void loadAllData();
    static void unloadAllData();

private:
    // In-memory cache: Filename -> Data (List of rows)
    static QMap<QString, QVector<QStringList>> m_cache;
};
