#include "../include/ui_habits.hpp"
#include "../include/timer.hpp"
#include "../include/notifications.hpp"
#include "../include/csvhandler.hpp"
#include "../include/exceptions.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>
#include <QPainter>
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>

namespace
{
    class PrayerChartWidget : public QWidget
    {
    public:
        PrayerChartWidget(const QMap<QDate, int> &data, QWidget *parent = nullptr)
            : QWidget(parent), m_data(data)
        {
            setMinimumHeight(250);
            setBackgroundRole(QPalette::Base);
            setAutoFillBackground(true);
        }

    protected:
        void paintEvent(QPaintEvent *) override
        {
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            int w = width();
            int h = height();
            int margin = 40;
            int xSpacing = (w - 2 * margin) / 7;
            int maxVal = 5;

            // Axes
            painter.setPen(QPen(Qt::black, 2));
            painter.drawLine(margin, h - margin, w - margin, h - margin);
            painter.drawLine(margin, h - margin, margin, margin);

            QDate today = QDate::currentDate();
            QVector<QPoint> points;

            for (int i = 6; i >= 0; --i)
            {
                QDate d = today.addDays(-i);
                int count = m_data.value(d, 0);

                int col = 6 - i;
                int x = margin + col * xSpacing + xSpacing / 2;
                int availableH = h - 2 * margin;
                int pointH = (double)count / maxVal * availableH;
                int y = h - margin - pointH;

                points.append(QPoint(x, y));

                painter.setPen(Qt::black);
                painter.drawText(QRect(x - xSpacing / 2, h - margin + 5, xSpacing, 20), Qt::AlignCenter, d.toString("dd/MM"));
            }

            // Draw connecting lines
            painter.setPen(QPen(QColor(100, 149, 237), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            if (points.size() > 1)
                painter.drawPolyline(points);

            // Draw points
            for (int i = 0; i < points.size(); ++i)
            {
                painter.setBrush(QColor(100, 149, 237));
                painter.setPen(Qt::white);
                painter.drawEllipse(points[i], 6, 6);

                int count = m_data.value(today.addDays(-(6 - i)), 0);
                if (count > 0)
                {
                    painter.setPen(Qt::black);
                    painter.drawText(QRect(points[i].x() - 20, points[i].y() - 30, 40, 20), Qt::AlignCenter, QString::number(count));
                }
            }
        }

    private:
        QMap<QDate, int> m_data;
    };
}

UIHabits::UIHabits(Ui::MainWindow *ui, AcadenceManager *manager, int uid, Timer *workoutTimer, QObject *parent)
    : QObject(parent), ui(ui), myManager(manager), userId(uid), m_workoutTimer(workoutTimer), activeTimerHabit(nullptr)
{
}

UIHabits::~UIHabits()
{
    qDeleteAll(currentHabitList);
}

void UIHabits::refreshHabits()
{
    DailyPrayerStatus prayers = myManager->getDailyPrayers(userId, QDate::currentDate().toString(Qt::ISODate));
    ui->chkFajr->setChecked(prayers.getFajr());
    ui->chkDhuhr->setChecked(prayers.getDhuhr());
    ui->chkAsr->setChecked(prayers.getAsr());
    ui->chkMaghrib->setChecked(prayers.getMaghrib());
    ui->chkIsha->setChecked(prayers.getIsha());

    qDeleteAll(currentHabitList);
    activeTimerHabit = nullptr;
    currentHabitList = myManager->getHabits(userId);
    ui->habitTableWidget->setRowCount(0);

    for (auto h : currentHabitList)
    {
        int row = ui->habitTableWidget->rowCount();
        ui->habitTableWidget->insertRow(row);

        ui->habitTableWidget->setItem(row, 0, new QTableWidgetItem(h->getTypeString()));
        ui->habitTableWidget->setItem(row, 1, new QTableWidgetItem(h->name));
        ui->habitTableWidget->setItem(row, 2, new QTableWidgetItem(h->getProgressString()));
        ui->habitTableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(h->streak)));
        QTableWidgetItem *statusItem = new QTableWidgetItem(h->isCompleted ? "Completed" : "Pending");
        if (h->isCompleted)
            statusItem->setForeground(Qt::green);
        ui->habitTableWidget->setItem(row, 4, statusItem);
    }
    ui->habitTableWidget->adjustColumnWidths();
}

void UIHabits::onAddHabitClicked()
{
    QStringList types = {"Duration (Timer)", "Count (Counter)", "Workout (Both)"};
    bool ok;
    QString typeStr = QInputDialog::getItem(nullptr, "Create Habit", "Type:", types, 0, false, &ok);
    if (!ok)
        return;

    QString name = QInputDialog::getText(nullptr, "Create Habit", "Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty())
    {
        Notifications::warning(nullptr, "Habit name cannot be empty.");
        return;
    }

    QStringList freqs = {"Daily", "Weekly"};
    QString freqStr = QInputDialog::getItem(nullptr, "Create Habit", "Frequency:", freqs, 0, false, &ok);
    Frequency f = (freqStr == "Daily") ? Frequency::DAILY : Frequency::WEEKLY;

    if (typeStr.startsWith("Duration"))
    {
        int target = QInputDialog::getInt(nullptr, "Create Habit", "Target Minutes:", 30, 1, 1440, 1, &ok);
        if (!ok)
            return;
        DurationHabit h(0, userId, name, f, target);
        myManager->addHabit(&h);
        Notifications::success(nullptr, "Habit \"" + name + "\" created successfully!");
        refreshHabits();
    }
    else if (typeStr.startsWith("Count"))
    {
        int target = QInputDialog::getInt(nullptr, "Create Habit", "Target Count:", 5, 1, 1000, 1, &ok);
        if (!ok)
            return;
        QString unit = QInputDialog::getText(nullptr, "Create Habit", "Unit:", QLineEdit::Normal, "", &ok);
        CountHabit h(0, userId, name, f, target, unit);
        myManager->addHabit(&h);
        Notifications::success(nullptr, "Habit \"" + name + "\" created successfully!");
        refreshHabits();
    }
    else if (typeStr.startsWith("Workout"))
    {
        int targetMin = QInputDialog::getInt(nullptr, "Create Habit", "Target Minutes:", 30, 1, 1440, 1, &ok);
        if (!ok)
            return;

        int targetCnt = QInputDialog::getInt(nullptr, "Create Habit", "Target Count/Reps:", 10, 1, 1000, 1, &ok);
        if (!ok)
            return;

        QString unit = QInputDialog::getText(nullptr, "Create Habit", "Unit:", QLineEdit::Normal, "", &ok);
        if (!ok)
            return;

        WorkoutHabit h(0, userId, name, f, targetMin, targetCnt, unit);
        myManager->addHabit(&h);
        Notifications::success(nullptr, "Habit \"" + name + "\" created successfully!");
        refreshHabits();
    }
}

void UIHabits::onPerformHabitClicked()
{
    int row = ui->habitTableWidget->currentRow();
    if (row < 0 || row >= currentHabitList.size())
    {
        Notifications::warning(nullptr, "Please select a habit to perform.");
        return;
    }

    Habit *h = currentHabitList[row];
    bool changed = false;

    if (auto wh = dynamic_cast<WorkoutHabit *>(h))
    {
        activeTimerHabit = wh;
        ui->groupBox_workoutTimer->setTitle("Workout: " + wh->name);

        double remaining = wh->targetMinutes - wh->currentMinutes;
        if (remaining <= 0.001)
            remaining = wh->targetMinutes;

        int totalSeconds = static_cast<int>(remaining * 60);
        ui->label_workoutTimerDisplay->setText(QString("%1:%2").arg(totalSeconds / 60, 2, 10, QChar('0')).arg(totalSeconds % 60, 2, 10, QChar('0')));

        Notifications::info(nullptr, "Timer set for '" + wh->name + "'.\nDon't forget to log your reps manually if needed!");
    }
    else if (auto dh = dynamic_cast<DurationHabit *>(h))
    {
        activeTimerHabit = dh;
        ui->groupBox_workoutTimer->setTitle("Stopwatch: " + dh->name);
        ui->label_workoutTimerDisplay->setText("00:00.00");

        Notifications::info(nullptr, "Stopwatch ready for '" + dh->name + "'.\nClick Start to begin tracking.");
    }
    else if (auto ch = dynamic_cast<CountHabit *>(h))
    {
        bool ok;
        int count = QInputDialog::getInt(nullptr, "Perform Habit", "Add Count:", 1, 1, 100, 1, &ok);
        if (ok)
        {
            ch->currentCount += count;
            if (ch->currentCount >= ch->targetCount)
            {
                ch->markComplete();
                Notifications::success(nullptr, "Habit completed! Great job on reaching your goal!");
            }
            else
            {
                int remaining = ch->targetCount - ch->currentCount;
                Notifications::info(nullptr, QString::number(remaining) + " " + ch->unit + " remaining.");
            }
            changed = true;
        }
    }

    if (changed)
    {
        myManager->updateHabit(h);
        refreshHabits();
    }
}

void UIHabits::onDeleteHabitClicked()
{
    int row = ui->habitTableWidget->currentRow();
    if (row < 0 || row >= currentHabitList.size())
    {
        Notifications::warning(nullptr, "Please select a habit to delete.");
        return;
    }

    Habit *habit = currentHabitList[row];
    if (Notifications::confirmDelete(nullptr, "habit \"" + habit->name + "\""))
    {
        int id = habit->id;
        myManager->deleteHabit(id);
        Notifications::success(nullptr, "Habit deleted successfully.");
        refreshHabits();
    }
}

void UIHabits::onFajrToggled(bool checked)
{
    myManager->updateDailyPrayer(userId, QDate::currentDate().toString(Qt::ISODate), "fajr", checked);
}

void UIHabits::onDhuhrToggled(bool checked)
{
    myManager->updateDailyPrayer(userId, QDate::currentDate().toString(Qt::ISODate), "dhuhr", checked);
}

void UIHabits::onAsrToggled(bool checked)
{
    myManager->updateDailyPrayer(userId, QDate::currentDate().toString(Qt::ISODate), "asr", checked);
}

void UIHabits::onMaghribToggled(bool checked)
{
    myManager->updateDailyPrayer(userId, QDate::currentDate().toString(Qt::ISODate), "maghrib", checked);
}

void UIHabits::onIshaToggled(bool checked)
{
    myManager->updateDailyPrayer(userId, QDate::currentDate().toString(Qt::ISODate), "isha", checked);
}

void UIHabits::onPrayerHistoryClicked()
{
    QMap<QDate, int> stats;
    try
    {
        QVector<QStringList> rows = CsvHandler::readCsv("prayers.csv");
        for (const auto &row : rows)
        {
            if (row.size() >= 7 && row[0].toInt() == userId)
            {
                QDate d = QDate::fromString(row[1], "yyyy-MM-dd");
                int count = row[2].toInt() + row[3].toInt() + row[4].toInt() + row[5].toInt() + row[6].toInt();
                stats[d] = count;
            }
        }
    }
    catch (const Acadence::Exception &e)
    {
        Notifications::error(nullptr, QString("Failed to load prayer history: %1").arg(e.what()));
        return;
    }

    QDialog dlg;
    dlg.setWindowTitle("Prayer History & Statistics");
    dlg.resize(1280, 720);
    QVBoxLayout *l = new QVBoxLayout(&dlg);
    l->addWidget(new QLabel("<h3>Last 7 Days Prayer Performance</h3>"));
    l->addWidget(new PrayerChartWidget(stats));
    QDialogButtonBox *bb = new QDialogButtonBox(QDialogButtonBox::Close);
    connect(bb, &QDialogButtonBox::rejected, &dlg, &QDialog::accept);
    l->addWidget(bb);
    dlg.exec();
}
