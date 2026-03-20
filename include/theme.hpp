#pragma once

#include <QString>
#include <QVector>
#include <QApplication>
#include <QWidget>

class QPropertyAnimation;
class QMouseEvent;
class QPaintEvent;

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
    inline QString Family = "Product Sans"; // Exposes font setting dynamically initialized at load
    inline QString TimerFamily = "Product Sans";

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

/** Renders an animated custom shape overriding standard system widgets */
class ThemeToggle : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal offset READ offset WRITE setOffset)

signals:
    void toggled(bool isDark);

public:
    explicit ThemeToggle(QWidget *parent = nullptr);

    void setDark(bool dark);
    bool isDark() const { return m_dark; }

    qreal offset() const { return m_offset; }
    void setOffset(qreal v);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_dark = false;
    qreal m_offset = 3.0;
    QPropertyAnimation *m_anim;
};
