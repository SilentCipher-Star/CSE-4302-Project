#pragma once
#include <QString>

class QTableView;

class Utils
{
public:
    static QString validatePassword(const QString &password);
    static QString validateUsername(const QString &username);
    static void loadFonts();
    static void adjustColumnWidths(QTableView *table);
};
