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

class ThemeManager
{
public:
    static void applyTheme(QApplication &a, const AppTheme &theme);

    static QVector<AppTheme> getAvailableThemes();
};

#endif // THEME_HPP