#pragma once

#include <QWidget>
#include <QPainter>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include "theme.hpp"

// Animated sun/moon toggle switch that replaces the plain "Dark Mode" button
class ThemeToggle : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal offset READ offset WRITE setOffset)

signals:
    void toggled(bool isDark);

public:
    explicit ThemeToggle(QWidget *parent = nullptr)
        : QWidget(parent), m_anim(new QPropertyAnimation(this, "offset", this))
    {
        setFixedSize(68, 30);
        setCursor(Qt::PointingHandCursor);
        setToolTip("Toggle dark / light mode");
        m_anim->setDuration(180);
        m_anim->setEasingCurve(QEasingCurve::InOutCubic);
    }

    void setDark(bool dark)
    {
        m_dark = dark;
        m_offset = dark ? 40.0 : 3.0;
        update();
    }

    bool isDark() const { return m_dark; }

    qreal offset() const { return m_offset; }
    void setOffset(qreal v)
    {
        m_offset = v;
        update();
    }

protected:
    void mousePressEvent(QMouseEvent *) override
    {
        m_dark = !m_dark;
        m_anim->setStartValue(m_offset);
        m_anim->setEndValue(m_dark ? 40.0 : 3.0);
        m_anim->start();
        emit toggled(m_dark);
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        // ── Pill background ──
        QColor pill = m_dark ? QColor(67, 56, 202) : QColor(251, 146, 60);
        p.setBrush(pill);
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(rect(), 15, 15);

        // ── Sliding circle ──
        p.setBrush(Qt::white);
        p.drawEllipse(QRectF(m_offset, 3, 24, 24));

        // ── Icon inside circle ──
        p.setPen(m_dark ? QColor(67, 56, 202) : QColor(234, 88, 12));
        QFont f(AppFonts::Family);
        f.setPixelSize(AppFonts::Small);
        p.setFont(f);
        p.drawText(QRectF(m_offset, 3, 24, 24), Qt::AlignCenter,
                   m_dark ? QString("\u263D") : QString("\u2600"));
    }

private:
    bool m_dark = false;
    qreal m_offset = 3.0;
    QPropertyAnimation *m_anim;
};
