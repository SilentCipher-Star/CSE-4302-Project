#pragma once

#include <QString>

class DatabaseManager
{
private:
    DatabaseManager() = default;

public:
    static DatabaseManager &instance();

    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    void initialize();
    bool backupCsvData(const QString &targetDirectory) const;
};
