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

namespace AppFonts
{
    inline const QString Family = "Product Sans";

    enum
    {
        Tiny = 12,
        Small = 16,
        Normal = 20,
        Large = 36,
        Title = 40,
        Timer = 120
    };
}

class ThemeManager
{
public:
    static void applyTheme(QApplication &a, const AppTheme &theme);

    static QVector<AppTheme> getAvailableThemes();
    static QVector<AppTheme> getDarkThemes();
};

#endif // THEME_HPP