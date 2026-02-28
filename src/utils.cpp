#include "../include/utils.hpp"
#include <QRegularExpression>
#include <QCoreApplication>
#include <QDir>
#include <QFontDatabase>

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