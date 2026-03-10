#pragma once

#include <QWidget>
#include <QListWidgetItem>
#include "appmanager.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

// Study Planner module: manages student tasks
class UIPlanner : public QObject
{
    Q_OBJECT

public:
    UIPlanner(Ui::MainWindow *ui, AcadenceManager *manager, int uid, QObject *parent = nullptr);

    void refreshPlanner();
    void setupConnections();

public slots:
    void onAddTaskClicked();
    void onDeleteTaskClicked();
    void onClearCompletedTasksClicked();
    void onTaskItemChanged(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    AcadenceManager *myManager;
    int userId;
};
