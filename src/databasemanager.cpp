#include "../include/databasemanager.hpp"
#include "../include/csvhandler.hpp"
#include <QDir>
#include <QFile>

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager manager;
    return manager;
}

void DatabaseManager::initialize()
{
    CsvHandler::initialize();
}

bool DatabaseManager::backupCsvData(const QString &targetDirectory) const
{
    const QString sourceDirPath = CsvHandler::getDataDirectory();
    QDir sourceDir(sourceDirPath);
    QDir targetDir(targetDirectory);

    if (!targetDir.exists() && !targetDir.mkpath("."))
    {
        return false;
    }

    const QStringList csvFiles = sourceDir.entryList(QStringList() << "*.csv", QDir::Files);
    for (const QString &fileName : csvFiles)
    {
        const QString sourceFile = sourceDir.filePath(fileName);
        const QString targetFile = targetDir.filePath(fileName);

        if (QFile::exists(targetFile) && !QFile::remove(targetFile))
        {
            return false;
        }

        if (!QFile::copy(sourceFile, targetFile))
        {
            return false;
        }
    }

    return true;
}
