#include "../include/ui_habits.hpp"
#include "../include/timer.hpp"
#include "../include/notifications.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>

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
    ui->habitTableWidget->resizeColumnsToContents();
    ui->habitTableWidget->horizontalHeader()->setStretchLastSection(true);
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
