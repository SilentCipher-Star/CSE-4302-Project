#include "../include/utils.hpp"
#include "../include/csvhandler.hpp"
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
    QString fontDir = QCoreApplication::applicationDirPath() + "/../fonts";
    QDir dir(fontDir);
    if (dir.exists())
    {
        QStringList filters;
        filters << "*.ttf" << "*.otf";
        dir.setNameFilters(filters);
        for (const QFileInfo &info : dir.entryInfoList())
        {
            QFontDatabase::addApplicationFont(info.absoluteFilePath());
        }
    }
}

void Utils::adjustColumnWidths(QTableView *table)
{
    if (!table || !table->model())
        return;

    table->resizeColumnsToContents();

    int colCount = table->model()->columnCount();
    int totalWidth = 0;

    QVector<int> columnWidths(colCount);
    for (int i = 0; i < colCount; ++i)
    {
        columnWidths[i] = table->columnWidth(i);
        totalWidth += columnWidths[i];
    }

    if (totalWidth > 0)
    {
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        int targetWidth = table->width() * 0.95;
        int usedWidth = 0;
        for (int i = 0; i < colCount; ++i)
        {
            int newWidth;
            if (i == colCount - 1)
                newWidth = targetWidth - usedWidth;
            else
                newWidth = int((double)columnWidths[i] / totalWidth * targetWidth);

            table->setColumnWidth(i, newWidth);
            usedWidth += newWidth;
        }
        table->horizontalHeader()->setStretchLastSection(true);
    }
}

QDate Utils::getDateForDay(QString dayName)
{
    QDate today = QDate::currentDate();
    int currentDayOfWeek = today.dayOfWeek(); // 1=Mon, 7=Sun

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
