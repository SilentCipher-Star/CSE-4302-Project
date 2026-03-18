#include "../include/ui_planner.hpp"
#include "../include/notifications.hpp"
#include "../include/commands.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>

UIPlanner::UIPlanner(Ui::MainWindow *ui, AcadenceManager *manager, int uid, QObject *parent)
    : QObject(parent), ui(ui), myManager(manager), userId(uid)
{
}

void UIPlanner::setupConnections()
{
    ui->addTaskButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->btnDeleteTask->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->btnClearCompletedTasks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void UIPlanner::refreshPlanner()
{
    ui->taskListWidget->blockSignals(true);
    ui->taskListWidget->clear();
    QVector<Task> tasks = myManager->getTasks(userId);
    for (const auto &t : tasks)
    {
        QListWidgetItem *item = new QListWidgetItem(t.getDescription());
        item->setData(Qt::UserRole, t.getId());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(t.getIsCompleted() ? Qt::Checked : Qt::Unchecked);

        if (t.getIsCompleted())
        {
            QFont f = item->font();
            f.setStrikeOut(true);
            item->setFont(f);
            item->setForeground(Qt::gray);
        }
        ui->taskListWidget->addItem(item);
    }
    ui->taskListWidget->blockSignals(false);
}

void UIPlanner::onAddTaskClicked()
{
    QString desc = ui->taskLineEdit->text().trimmed();
    if (desc.isEmpty())
    {
        Notifications::warning(nullptr, "Please enter a task description.");
        return;
    }

    if (desc.length() > 200)
    {
        Notifications::warning(nullptr, "Task description is too long (max 200 characters).");
        return;
    }

    auto cmd = std::make_shared<AddTaskCommand>(userId, desc);
    myManager->executeCommand(cmd);
    ui->taskLineEdit->clear();
    Notifications::success(nullptr, "Task added successfully.");
}

void UIPlanner::onDeleteTaskClicked()
{
    QListWidgetItem *item = ui->taskListWidget->currentItem();
    if (!item)
    {
        Notifications::warning(nullptr, "Please select a task to delete.");
        return;
    }

    if (Notifications::confirmDelete(nullptr, "this task"))
    {
        int id = item->data(Qt::UserRole).toInt();
        // Capture task state for undo
        QString desc = item->text();
        bool wasCompleted = (item->checkState() == Qt::Checked);
        auto cmd = std::make_shared<DeleteTaskCommand>(id, userId, desc, wasCompleted);
        myManager->executeCommand(cmd);
        Notifications::success(nullptr, "Task deleted successfully.");
    }
}

void UIPlanner::onClearCompletedTasksClicked()
{
    if (Notifications::confirm(nullptr, "Clear Completed Tasks", "Delete all completed tasks?"))
    {
        auto cmd = std::make_shared<DeleteCompletedTasksCommand>(userId);
        myManager->executeCommand(cmd);
        Notifications::success(nullptr, "Completed tasks cleared.");
    }
}

void UIPlanner::onTaskItemChanged(QListWidgetItem *item)
{
    int id = item->data(Qt::UserRole).toInt();
    bool isChecked = (item->checkState() == Qt::Checked);
    auto cmd = std::make_shared<CompleteTaskCommand>(id, !isChecked, isChecked);
    myManager->executeCommand(cmd);
}
