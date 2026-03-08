#pragma once
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpinBox>
#include <QProgressBar>
#include <QPushButton>
#include <QFrame>
#include <QVector>
#include <QString>
#include <QFont>
#include <cmath>

struct AttendanceSimRow
{
    QString  courseName;
    int      attended;
    int      total;
    QLabel  *lblProjected;
    QLabel  *lblSafe;
    QLabel  *lblStatus;
    QProgressBar *bar;
    QSpinBox     *spinSkip;
};

class AttendanceSimulatorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AttendanceSimulatorDialog(
        const QVector<QPair<QString, QPair<int,int>>> &courseData,
        QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowTitle("Attendance Risk Simulator");
        setMinimumWidth(620);
        setMinimumHeight(420);

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(10);

        QLabel *title = new QLabel("Attendance Risk Simulator");
        title->setStyleSheet("font-size:16px; font-weight:bold; padding:6px 0;");
        title->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(title);

        QLabel *hint = new QLabel(
            "Use the spinner to simulate skipping future classes.\n"
            "The bar updates instantly to show your projected attendance.");
        hint->setStyleSheet("color:#888; font-size:11px;");
        hint->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(hint);

        QFrame *sep = new QFrame();
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet("color:#444;");
        mainLayout->addWidget(sep);

        QScrollArea *scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);

        QWidget *container = new QWidget();
        QVBoxLayout *rows  = new QVBoxLayout(container);
        rows->setSpacing(12);
        rows->setContentsMargins(8, 8, 8, 8);

        for (const auto &entry : courseData)
        {
            AttendanceSimRow row;
            row.courseName = entry.first;
            row.attended   = entry.second.first;
            row.total      = entry.second.second;

            QFrame *card = new QFrame();
            card->setFrameShape(QFrame::StyledPanel);
            card->setStyleSheet("QFrame { border-radius:6px; padding:6px; }");

            QVBoxLayout *cardLayout = new QVBoxLayout(card);
            cardLayout->setSpacing(6);

            QHBoxLayout *topRow = new QHBoxLayout();
            QLabel *lblName = new QLabel(row.courseName);
            lblName->setStyleSheet("font-weight:bold; font-size:13px;");
            topRow->addWidget(lblName);
            topRow->addStretch();

            double curPct = (row.total > 0)
                ? (double)row.attended / row.total * 100.0 : 0.0;
            QLabel *lblCur = new QLabel(
                QString("Current: %1% (%2/%3 classes)")
                    .arg(QString::number(curPct,'f',1))
                    .arg(row.attended)
                    .arg(row.total));
            lblCur->setStyleSheet("font-size:11px; color:#aaa;");
            topRow->addWidget(lblCur);
            cardLayout->addLayout(topRow);

            row.bar = new QProgressBar();
            row.bar->setRange(0, 100);
            row.bar->setValue((int)curPct);
            row.bar->setTextVisible(false);
            row.bar->setFixedHeight(14);
            applyBarStyle(row.bar, curPct);
            cardLayout->addWidget(row.bar);

            QHBoxLayout *midRow = new QHBoxLayout();

            row.lblProjected = new QLabel(
                QString("Projected: %1%").arg(QString::number(curPct,'f',1)));
            row.lblProjected->setStyleSheet("font-size:12px; font-weight:bold;");
            midRow->addWidget(row.lblProjected);

            midRow->addStretch();

            row.lblStatus = new QLabel();
            row.lblStatus->setStyleSheet("font-size:12px; font-weight:bold;");
            midRow->addWidget(row.lblStatus);

            cardLayout->addLayout(midRow);

            QHBoxLayout *botRow = new QHBoxLayout();

            row.lblSafe = new QLabel();
            row.lblSafe->setStyleSheet("font-size:11px;");
            botRow->addWidget(row.lblSafe);

            botRow->addStretch();

            QLabel *lblSkipLabel = new QLabel("Extra classes to skip:");
            lblSkipLabel->setStyleSheet("font-size:11px;");
            botRow->addWidget(lblSkipLabel);

            row.spinSkip = new QSpinBox();
            row.spinSkip->setRange(0, qMax(1, row.total * 3));
            row.spinSkip->setValue(0);
            row.spinSkip->setFixedWidth(70);
            botRow->addWidget(row.spinSkip);

            cardLayout->addLayout(botRow);

            rows->addWidget(card);
            m_rows.append(row);
            updateRow(m_rows.size() - 1, 0);

            int idx = m_rows.size() - 1;
            connect(row.spinSkip, QOverload<int>::of(&QSpinBox::valueChanged),
                    this, [this, idx](int val){ updateRow(idx, val); });
        }

        rows->addStretch();
        scroll->setWidget(container);
        mainLayout->addWidget(scroll, 1);

        QPushButton *btnClose = new QPushButton("Close");
        btnClose->setFixedWidth(100);
        QHBoxLayout *btnRow = new QHBoxLayout();
        btnRow->addStretch();
        btnRow->addWidget(btnClose);
        mainLayout->addLayout(btnRow);
        connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    }

private:
    QVector<AttendanceSimRow> m_rows;

    void updateRow(int idx, int extraSkip)
    {
        auto &row = m_rows[idx];

        int newTotal      = row.total + extraSkip;
        double projected  = (newTotal > 0)
            ? (double)row.attended / newTotal * 100.0 : 0.0;

        row.lblProjected->setText(
            QString("Projected: %1%").arg(QString::number(projected,'f',1)));

        row.bar->setValue(qMin(100, (int)projected));
        applyBarStyle(row.bar, projected);

        QString statusText;
        QString statusColor;
        if (projected >= 75.0)
        {
            statusText  = "Safe";
            statusColor = "#22a85a";
        }
        else if (projected >= 60.0)
        {
            statusText  = "At Risk";
            statusColor = "#e89020";
        }
        else
        {
            statusText  = "Critical";
            statusColor = "#dc143c";
        }
        row.lblStatus->setText(statusText);
        row.lblStatus->setStyleSheet(
            QString("font-size:12px; font-weight:bold; color:%1;").arg(statusColor));

        int safeSkip = 0;
        if (row.total > 0)
        {
            double maxMissable = row.attended / 0.75 - row.total;
            safeSkip = (int)std::floor(maxMissable);
            if (safeSkip < 0) safeSkip = 0;
        }

        if (extraSkip == 0)
        {
            if (safeSkip > 0)
                row.lblSafe->setText(
                    QString("You can safely skip %1 more class(es)").arg(safeSkip));
            else
                row.lblSafe->setText("Cannot skip any more classes safely");
        }
        else if (projected >= 75.0)
        {
            row.lblSafe->setText(
                QString("Still safe after skipping %1 class(es)").arg(extraSkip));
        }
        else
        {
            double needed = std::ceil(0.75 * newTotal - row.attended);
            row.lblSafe->setText(
                QString("Need %1 more attendance(s) to recover above 75%").arg((int)needed));
        }
    }

    void applyBarStyle(QProgressBar *bar, double pct)
    {
        QString color;
        if (pct >= 75.0)
            color = "#22a85a";
        else if (pct >= 60.0)
            color = "#e89020";
        else
            color = "#dc143c";

        bar->setStyleSheet(QString(
            "QProgressBar { background:#2a2d3e; border-radius:5px; }"
            "QProgressBar::chunk { background:%1; border-radius:5px; }").arg(color));
    }
};
