#include "csvhandler.hpp"
#include "exceptions.hpp"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>

QString CsvHandler::getDataDirectory()
{
    QString path = QCoreApplication::applicationDirPath() + "/../data";
    QDir dir(path);
    if (!dir.exists())
    {
        dir.mkpath(".");
    }
    return path + "/";
}

QVector<QStringList> CsvHandler::readCsv(const QString &filename)
{
    QVector<QStringList> data;
    QString fullPath = getDataDirectory() + filename;
    QFile file(fullPath);

    if (!file.exists())
    {
        return data; // Return empty if file doesn't exist yet
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        throw Acadence::FileException("Failed to open file for reading: " + filename);
    }
    else
    {
        QTextStream in(&file);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (line.trimmed().isEmpty())
                continue;

            QStringList row;
            QString currentField;
            bool inQuotes = false;
            for (int i = 0; i < line.length(); ++i)
            {
                QChar c = line[i];
                if (c == '"')
                {
                    if (inQuotes && i + 1 < line.length() && line[i + 1] == '"')
                    {
                        currentField += '"';
                        i++;
                    }
                    else
                    {
                        inQuotes = !inQuotes;
                    }
                }
                else if (c == ',' && !inQuotes)
                {
                    row.append(currentField.trimmed());
                    currentField.clear();
                }
                else
                {
                    currentField += c;
                }
            }
            row.append(currentField.trimmed());
            data.append(row);
        }
        file.close();
    }
    return data;
}

static QString escapeCsv(const QString &val)
{
    if (val.contains(',') || val.contains('"') || val.contains('\n'))
    {
        QString temp = val;
        temp.replace("\"", "\"\"");
        return "\"" + temp + "\"";
    }
    return val;
}

void CsvHandler::writeCsv(const QString &filename, const QVector<QStringList> &data)
{
    QFile file(getDataDirectory() + filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        throw Acadence::FileException("Failed to open file for writing: " + filename);
    }
    else
    {
        QTextStream out(&file);
        for (const auto &row : data)
        {
            QStringList escapedRow;
            for (const QString &field : row)
                escapedRow << escapeCsv(field);
            out << escapedRow.join(",") << "\n";
        }
        file.close();
    }
}

void CsvHandler::appendCsv(const QString &filename, const QStringList &fields)
{
    QFile file(getDataDirectory() + filename);
    if (!file.open(QIODevice::Append | QIODevice::Text))
    {
        throw Acadence::FileException("Failed to open file for appending: " + filename);
    }
    else
    {
        QTextStream out(&file);
        QStringList escaped;
        for (const QString &f : fields)
            escaped << escapeCsv(f);
        out << escaped.join(",") << "\n";
        file.close();
    }
}