#include "../include/utils.hpp"
#include "../include/csvhandler.hpp"
#include "../include/theme.hpp"
#include <QRegularExpression>
#include <QStandardItemModel>
#include <QCoreApplication>
#include <QDir>
#include <QFontDatabase>
#include <QTableView>
#include <QHeaderView>
#include <QVector>

QString Utils::validatePassword(const QString &password)
{
    if (password.length() < 6)
    {
        return "Password must be at least 6 characters long.";
    }

    bool hasDigit = false;
    bool hasUpper = false;
    for (const QChar &c : password)
    {
        if (c.isDigit())
            hasDigit = true;
        if (c.isUpper())
            hasUpper = true;
    }

    if (!hasDigit)
        return "Password must contain at least one number.";
    if (!hasUpper)
        return "Password must contain at least one uppercase letter.";

    return "";
}

QString Utils::validateUsername(const QString &username)
{
    if (username.trimmed().isEmpty())
    {
        return "Username cannot be empty.";
    }
    QRegularExpression re("^[a-zA-Z0-9_]+$");
    if (!re.match(username).hasMatch())
    {
        return "Username can only contain letters, numbers, and underscores.";
    }
    return "";
}

void Utils::loadFonts()
{
    QString fontDir = QCoreApplication::applicationDirPath() + "/../assets/fonts";
    QDir dir(fontDir);
    if (dir.exists())
    {
        QStringList filters;
        filters << "*.ttf" << "*.otf";
        dir.setNameFilters(filters);
        int loadedCount = 0;
        for (const QFileInfo &info : dir.entryInfoList())
        {
            int fontId = QFontDatabase::addApplicationFont(info.absoluteFilePath());
            if (fontId != -1)
            {
                QStringList families = QFontDatabase::applicationFontFamilies(fontId);
                if (!families.isEmpty())
                {
                    if (loadedCount == 0)
                    {
                        AppFonts::Family = families.first();
                    }
                    else if (loadedCount == 1)
                    {
                        AppFonts::TimerFamily = families.first();
                    }
                    loadedCount++;
                }
            }
        }
        if (loadedCount == 1)
        {
            AppFonts::TimerFamily = AppFonts::Family;
        }
    }
}

QDate Utils::getDateForDay(QString dayName)
{
    QDate today = QDate::currentDate();
    int currentDayOfWeek = today.dayOfWeek(); // Qt numeric bounds: 1=Mon, 7=Sun

    QStringList days = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    int targetDayOfWeek = days.indexOf(dayName) + 1;

    int diff = targetDayOfWeek - currentDayOfWeek;
    return today.addDays(diff);
}

void Utils::saveTableData(QStandardItemModel *model, const QString &tableName)
{
    QVector<QStringList> data;
    for (int i = 0; i < model->rowCount(); ++i)
    {
        QStringList rowData;
        for (int j = 0; j < model->columnCount(); ++j)
        {
            QStandardItem *item = model->item(i, j);
            rowData << (item ? item->text() : "");
        }
        data.append(rowData);
    }
    CsvHandler::writeCsv(tableName + ".csv", data);
}
