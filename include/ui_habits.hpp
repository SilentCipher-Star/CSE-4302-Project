#pragma once

#include <QWidget>
#include "appmanager.hpp"
#include "habit.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class Timer;

// Habits & Prayers module: manages habit tracking and daily prayer tracking
class UIHabits : public QObject
{
    Q_OBJECT

public:
    UIHabits(Ui::MainWindow *ui, AcadenceManager *manager, int uid, Timer *workoutTimer, QObject *parent = nullptr);
    ~UIHabits();

    void refreshHabits();
    void setActiveTimerHabit(DurationHabit *habit) { activeTimerHabit = habit; }
    DurationHabit *getActiveTimerHabit() const { return activeTimerHabit; }

public slots:
    void onAddHabitClicked();
    void onPerformHabitClicked();
    void onDeleteHabitClicked();
    void onFajrToggled(bool checked);
    void onDhuhrToggled(bool checked);
    void onAsrToggled(bool checked);
    void onMaghribToggled(bool checked);
    void onIshaToggled(bool checked);

private:
    Ui::MainWindow *ui;
    AcadenceManager *myManager;
    int userId;
    Timer *m_workoutTimer;
    QVector<Habit *> currentHabitList;
    DurationHabit *activeTimerHabit;
};
