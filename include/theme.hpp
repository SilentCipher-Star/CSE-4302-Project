#ifndef THEME_HPP
#define THEME_HPP

#include <QString>
#include <QVector>
#include <QApplication>

struct AppTheme
{
    QString name;
    QString background;
    QString surface;
    QString text;
    QString accent;
};

struct AppFonts
{
    static const QString Normal;
    static const QString Small;
    static const QString Large;
    static const QString Timer;
    static const QString Title;
};

class ThemeManager
{
public:
    static void applyTheme(QApplication &a, const AppTheme &theme);

    static QVector<AppTheme> getAvailableThemes();
    static QVector<AppTheme> getDarkThemes();
};

#endif // THEME_HPP