#include "../include/ui_timers.hpp"
#include "../include/ui_habits.hpp"
#include "../include/timer.hpp"
#include "../include/theme.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>

UITimers::UITimers(Ui::MainWindow *ui, AcadenceManager *manager, int uid, QObject *parent)
    : QObject(parent), ui(ui), myManager(manager), userId(uid), m_habitsModule(nullptr)
{
}

UITimers::~UITimers()
{
}

void UITimers::setupTimers()
{
    // Focus Timer
    ui->label_timerDisplay->setText("00:00");
    ui->label_timerDisplay->setAlignment(Qt::AlignCenter);
    m_focusTimer = new Timer(this);

    auto updateDisplay = [](QLabel *lbl, const QString &time, float progress)
    {
        lbl->setText(time);
        QColor acc = lbl->palette().color(QPalette::Highlight);
        QString accRgba = QString("rgba(%1, %2, %3, 0.25)").arg(acc.red()).arg(acc.green()).arg(acc.blue());

        QString style = QString("QLabel { font-size: %1; border: 3px solid palette(highlight); border-radius: 16px; color: palette(text); "
                                "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                                "stop:0 %2, stop:%3 %2, "
                                "stop:%3 transparent, stop:1 transparent); }")
                            .arg(AppFonts::Timer)
                            .arg(accRgba)
                            .arg(progress > 0.001 ? progress - 0.001 : 0)
                            .arg(progress);
        lbl->setStyleSheet(style);
    };

    connect(m_focusTimer, &Timer::timeUpdated, [this, updateDisplay](QString time, float progress)
            { updateDisplay(ui->label_timerDisplay, time, progress); });

    connect(m_focusTimer, &Timer::finished, [this]()
            { QMessageBox::information(nullptr, "Timer", "Focus session complete!"); });

    // Workout/Habit Timer
    ui->label_workoutTimerDisplay->setText("00:00");
    ui->label_workoutTimerDisplay->setAlignment(Qt::AlignCenter);
    m_workoutTimer = new Timer(this);

    connect(m_workoutTimer, &Timer::timeUpdated, [this, updateDisplay](QString time, float progress)
            { updateDisplay(ui->label_workoutTimerDisplay, time, progress); });

    connect(m_workoutTimer, &Timer::finished, [this]()
            {
        QMessageBox::information(nullptr, "Habit", "Session complete!");
        if (m_habitsModule && m_habitsModule->getActiveTimerHabit()) {
            DurationHabit *activeTimerHabit = m_habitsModule->getActiveTimerHabit();
            if (auto wh = dynamic_cast<WorkoutHabit*>(activeTimerHabit)) {
                bool ok;
                int count = QInputDialog::getInt(nullptr, "Workout Progress", "Session Done! Add Reps/Count:", 0, 0, 1000, 1, &ok);
                if (ok && count > 0) {
                    wh->currentCount += count;
                }
                wh->currentMinutes = wh->targetMinutes;
                if (wh->currentCount >= wh->targetCount)
                    wh->markComplete();
                myManager->updateHabit(wh);
            } else if (auto dh = dynamic_cast<DurationHabit*>(activeTimerHabit)) {
                dh->currentMinutes = dh->targetMinutes;
                dh->markComplete();
                myManager->updateHabit(dh);
            }
            m_habitsModule->refreshHabits();
        } });
}

void UITimers::onFocusTimerStartClicked()
{
    m_focusTimer->start(ui->spinTimerMinutes->value());
    ui->btnTimerPause->setText("Pause");
}

void UITimers::onFocusTimerPauseClicked()
{
    m_focusTimer->pause();
    if (ui->btnTimerPause->text() == "Pause")
        ui->btnTimerPause->setText("Resume");
    else
        ui->btnTimerPause->setText("Pause");
}

void UITimers::onFocusTimerStopClicked()
{
    m_focusTimer->stop();
    ui->btnTimerPause->setText("Pause");
}

void UITimers::onWorkoutTimerStartClicked()
{
    if (!m_habitsModule || !m_habitsModule->getActiveTimerHabit())
    {
        QMessageBox::warning(nullptr, "Error", "Please select a habit to perform first.");
        return;
    }

    ui->btnWorkoutPause->setText("Pause");
    DurationHabit *activeTimerHabit = m_habitsModule->getActiveTimerHabit();

    if (auto wh = dynamic_cast<WorkoutHabit *>(activeTimerHabit))
    {
        double remaining = wh->targetMinutes - wh->currentMinutes;
        if (remaining <= 0.001)
            remaining = wh->targetMinutes;
        m_workoutTimer->start(remaining);
    }
    else if (auto dh = dynamic_cast<DurationHabit *>(activeTimerHabit))
    {
        double remaining = dh->targetMinutes - dh->currentMinutes;
        if (remaining <= 0.001)
            remaining = dh->targetMinutes;
        m_workoutTimer->startStopwatch(remaining);
    }
}

void UITimers::onWorkoutTimerPauseClicked()
{
    m_workoutTimer->pause();
    if (ui->btnWorkoutPause->text() == "Pause")
        ui->btnWorkoutPause->setText("Resume");
    else
        ui->btnWorkoutPause->setText("Pause");
}

void UITimers::onWorkoutTimerStopClicked()
{
    double elapsed = m_workoutTimer->getElapsedMinutes();
    m_workoutTimer->stop();
    ui->btnWorkoutPause->setText("Pause");

    if (m_habitsModule && m_habitsModule->getActiveTimerHabit())
    {
        DurationHabit *activeTimerHabit = m_habitsModule->getActiveTimerHabit();
        if (elapsed > 0)
        {
            activeTimerHabit->currentMinutes += elapsed;

            if (auto wh = dynamic_cast<WorkoutHabit *>(activeTimerHabit))
            {
                bool ok;
                int count = QInputDialog::getInt(nullptr, "Workout Stopped", "Add Reps/Count:", 0, 0, 1000, 1, &ok);
                if (ok && count > 0)
                    wh->currentCount += count;

                if (wh->currentMinutes >= wh->targetMinutes && wh->currentCount >= wh->targetCount)
                    wh->markComplete();
            }
            else if (activeTimerHabit->currentMinutes >= activeTimerHabit->targetMinutes)
                activeTimerHabit->markComplete();

            myManager->updateHabit(activeTimerHabit);
        }
    }
}
