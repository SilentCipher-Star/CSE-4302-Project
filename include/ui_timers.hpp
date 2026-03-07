#pragma once

#include <QWidget>
#include "appmanager.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class Timer;
class UIHabits;

// Timers module: manages focus timer and workout/habit timer
class UITimers : public QObject
{
    Q_OBJECT

public:
    UITimers(Ui::MainWindow *ui, AcadenceManager *manager, int uid, QObject *parent = nullptr);
    ~UITimers();

    void setupTimers();
    Timer *getFocusTimer() const { return m_focusTimer; }
    Timer *getWorkoutTimer() const { return m_workoutTimer; }
    void setHabitsModule(UIHabits *habitsModule) { m_habitsModule = habitsModule; }

public slots:
    void onFocusTimerStartClicked();
    void onFocusTimerPauseClicked();
    void onFocusTimerStopClicked();
    void onWorkoutTimerStartClicked();
    void onWorkoutTimerPauseClicked();
    void onWorkoutTimerStopClicked();

private:
    Ui::MainWindow *ui;
    AcadenceManager *myManager;
    int userId;
    Timer *m_focusTimer;
    Timer *m_workoutTimer;
    UIHabits *m_habitsModule;
};
