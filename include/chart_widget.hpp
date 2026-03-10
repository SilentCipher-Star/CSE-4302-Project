#pragma once

#include <QWidget>
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QVector>
#include <QPair>
#include <QString>
#include <QColor>

class BarChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BarChartWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumHeight(220);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setData(const QVector<QPair<QString, double>> &data, double maxValue = 100.0)
    {
        m_data    = data;
        m_maxValue = maxValue > 0 ? maxValue : 100.0;
        update();
    }

    void setBarColor(const QColor &color) { m_barColor = color; update(); }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);

        const int ml = 50, mr = 16, mt = 24, mb = 64;
        const int chartW = width()  - ml - mr;
        const int chartH = height() - mt - mb;

        p.fillRect(rect(), palette().window());

        if (m_data.isEmpty())
        {
            p.setPen(palette().text().color());
            p.drawText(rect(), Qt::AlignCenter, "No data available");
            return;
        }

        // Grid lines + Y labels
        p.setFont(QFont("Segoe UI", 8));
        for (int i = 0; i <= 4; ++i)
        {
            int y = mt + chartH - chartH * i / 4;
            p.setPen(QPen(palette().mid().color(), 1, Qt::DashLine));
            p.drawLine(ml, y, ml + chartW, y);

            p.setPen(palette().text().color());
            p.drawText(0, y - 8, ml - 6, 16, Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(m_maxValue * i / 4, 'f', 0) + "%");
        }

        // Bars
        const int n          = m_data.size();
        const int spacing    = 12;
        const int barW       = qMax(24, (chartW - spacing * (n + 1)) / n);

        for (int i = 0; i < n; ++i)
        {
            double val  = qBound(0.0, m_data[i].second, m_maxValue);
            int    barH = static_cast<int>(chartH * val / m_maxValue);
            int    x    = ml + spacing + i * (barW + spacing);
            int    y    = mt + chartH - barH;

            QLinearGradient grad(x, y, x, y + barH);
            grad.setColorAt(0, m_barColor.lighter(130));
            grad.setColorAt(1, m_barColor);

            QPainterPath path;
            path.addRoundedRect(x, y, barW, barH, 6, 6);
            p.fillPath(path, grad);

            // Value label
            p.setPen(palette().text().color());
            p.setFont(QFont("Segoe UI", 8, QFont::Bold));
            p.drawText(x, y - 18, barW, 16, Qt::AlignCenter,
                       QString::number(m_data[i].second, 'f', 1) + "%");

            // X label (rotated)
            p.save();
            p.translate(x + barW / 2, mt + chartH + 10);
            p.rotate(30);
            p.setFont(QFont("Segoe UI", 8));
            p.setPen(palette().text().color());
            p.drawText(0, 0, 130, 18, Qt::AlignLeft, m_data[i].first);
            p.restore();
        }

        // Axes
        p.setPen(QPen(palette().text().color(), 2));
        p.drawLine(ml, mt, ml, mt + chartH);
        p.drawLine(ml, mt + chartH, ml + chartW, mt + chartH);
    }

private:
    QVector<QPair<QString, double>> m_data;
    double m_maxValue = 100.0;
    QColor m_barColor{233, 30, 140};
};
