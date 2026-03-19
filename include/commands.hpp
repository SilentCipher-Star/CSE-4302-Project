#pragma once

#include "icommand.hpp"
#include "appmanager.hpp"
#include "manager_academics.hpp"
#include "manager_tasks.hpp"
#include "manager_notices.hpp"
#include "csvhandler.hpp"
#include <QVector>
#include <algorithm>

/**
 * Command Pattern - Concrete Commands
 *
 * Each class encapsulates one undoable operation.
 * Commands call the static Manager methods directly and
 * capture the old state needed for reversal.
 */

// ============================================================================
// MarkAttendanceCommand - Toggle a single student's attendance for a date
// ============================================================================
class MarkAttendanceCommand : public ICommand
{
public:
    MarkAttendanceCommand(int courseId, int studentId, const QString &date, bool newPresent)
        : m_courseId(courseId), m_studentId(studentId), m_date(date), m_newPresent(newPresent)
    {
        m_oldPresent = ManagerAcademics::isPresent(courseId, studentId, date);
    }

    void execute() override
    {
        ManagerAcademics::markAttendance(m_courseId, m_studentId, m_date, m_newPresent);
    }

    void undo() override
    {
        ManagerAcademics::markAttendance(m_courseId, m_studentId, m_date, m_oldPresent);
    }

    QString description() const override
    {
        return QString("Mark attendance for student %1").arg(m_studentId);
    }

    DataType affectedDataType() const override { return DataType::Academics; }

private:
    int m_courseId, m_studentId;
    QString m_date;
    bool m_newPresent, m_oldPresent;
};

// ============================================================================
// AddGradeCommand - Set or update a student's grade for an assessment
// ============================================================================
class AddGradeCommand : public ICommand
{
public:
    AddGradeCommand(int studentId, int assessmentId, double newMarks)
        : m_studentId(studentId), m_assessmentId(assessmentId), m_newMarks(newMarks)
    {
        m_oldMarks = ManagerAcademics::getGrade(studentId, assessmentId);
    }

    void execute() override
    {
        ManagerAcademics::addGrade(m_studentId, m_assessmentId, m_newMarks);
    }

    void undo() override
    {
        if (m_oldMarks < 0)
        {
            // Grade didn't exist before — remove the row
            QVector<QStringList> data = CsvHandler::readCsv("grades.csv");
            QVector<QStringList> filtered;
            for (const auto &row : data)
            {
                if (row.size() >= 3 && row[0].toInt() == m_studentId && row[1].toInt() == m_assessmentId)
                    continue;
                filtered.append(row);
            }
            CsvHandler::writeCsv("grades.csv", filtered);
        }
        else
        {
            ManagerAcademics::addGrade(m_studentId, m_assessmentId, m_oldMarks);
        }
    }

    QString description() const override
    {
        return QString("Set grade for student %1, assessment %2").arg(m_studentId).arg(m_assessmentId);
    }

    DataType affectedDataType() const override { return DataType::Academics; }

private:
    int m_studentId, m_assessmentId;
    double m_newMarks, m_oldMarks;
};

// ============================================================================
// AddTaskCommand - Add a new task for a student
// ============================================================================
class AddTaskCommand : public ICommand
{
public:
    AddTaskCommand(int userId, const QString &desc)
        : m_userId(userId), m_desc(desc), m_assignedId(-1) {}

    void execute() override
    {
        // Determine the ID that will be assigned
        QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
        int maxId = 0;
        for (const auto &row : data)
            if (!row.isEmpty())
                maxId = std::max(maxId, row[0].toInt());
        m_assignedId = maxId + 1;

        ManagerTasks::addTask(m_userId, m_desc);
    }

    void undo() override
    {
        if (m_assignedId > 0)
            ManagerTasks::deleteTask(m_assignedId);
    }

    QString description() const override
    {
        return QString("Add task: %1").arg(m_desc.left(40));
    }

    DataType affectedDataType() const override { return DataType::Tasks; }

private:
    int m_userId;
    QString m_desc;
    int m_assignedId;
};

// ============================================================================
// DeleteTaskCommand - Delete a task (captures full state for restoration)
// ============================================================================
class DeleteTaskCommand : public ICommand
{
public:
    DeleteTaskCommand(int taskId, int userId, const QString &desc, bool wasCompleted)
        : m_taskId(taskId), m_userId(userId), m_desc(desc), m_wasCompleted(wasCompleted) {}

    void execute() override
    {
        ManagerTasks::deleteTask(m_taskId);
    }

    void undo() override
    {
        // Re-insert with original data
        CsvHandler::appendCsv("tasks.csv", {
            QString::number(m_taskId),
            QString::number(m_userId),
            m_desc,
            m_wasCompleted ? "1" : "0"
        });
    }

    QString description() const override
    {
        return QString("Delete task: %1").arg(m_desc.left(40));
    }

    DataType affectedDataType() const override { return DataType::Tasks; }

private:
    int m_taskId, m_userId;
    QString m_desc;
    bool m_wasCompleted;
};

// ============================================================================
// CompleteTaskCommand - Toggle a task's completion status
// ============================================================================
class CompleteTaskCommand : public ICommand
{
public:
    CompleteTaskCommand(int taskId, bool oldStatus, bool newStatus)
        : m_taskId(taskId), m_oldStatus(oldStatus), m_newStatus(newStatus) {}

    void execute() override
    {
        ManagerTasks::completeTask(m_taskId, m_newStatus);
    }

    void undo() override
    {
        ManagerTasks::completeTask(m_taskId, m_oldStatus);
    }

    QString description() const override
    {
        return m_newStatus ? QString("Complete task %1").arg(m_taskId)
                           : QString("Uncomplete task %1").arg(m_taskId);
    }

    DataType affectedDataType() const override { return DataType::Tasks; }

private:
    int m_taskId;
    bool m_oldStatus, m_newStatus;
};

// ============================================================================
// DeleteCompletedTasksCommand - Batch delete all completed tasks
// ============================================================================
class DeleteCompletedTasksCommand : public ICommand
{
public:
    DeleteCompletedTasksCommand(int userId)
        : m_userId(userId)
    {
        // Capture all completed tasks before deletion
        QVector<Task> tasks = ManagerTasks::getTasks(userId);
        for (const auto &t : tasks)
        {
            if (t.getIsCompleted())
                m_deletedTasks.append({t.getId(), t.getDescription()});
        }
    }

    void execute() override
    {
        ManagerTasks::deleteCompletedTasks(m_userId);
    }

    void undo() override
    {
        // Re-insert all captured tasks
        for (const auto &t : m_deletedTasks)
        {
            CsvHandler::appendCsv("tasks.csv", {
                QString::number(t.first),
                QString::number(m_userId),
                t.second,
                "1" // they were completed
            });
        }
    }

    QString description() const override
    {
        return QString("Clear %1 completed task(s)").arg(m_deletedTasks.size());
    }

    DataType affectedDataType() const override { return DataType::Tasks; }

private:
    int m_userId;
    QVector<QPair<int, QString>> m_deletedTasks; // id, description
};

// ============================================================================
// AddNoticeCommand - Post a new notice
// ============================================================================
class AddNoticeCommand : public ICommand
{
public:
    AddNoticeCommand(const QString &content, const QString &author)
        : m_content(content), m_author(author)
    {
        m_date = QDate::currentDate().toString("yyyy-MM-dd");
    }

    void execute() override
    {
        ManagerNotices::addNotice(m_content, m_author);
    }

    void undo() override
    {
        ManagerNotices::deleteNotice(m_date, m_author, m_content);
    }

    QString description() const override
    {
        return QString("Post notice by %1").arg(m_author);
    }

    DataType affectedDataType() const override { return DataType::Notices; }

private:
    QString m_content, m_author, m_date;
};

// ============================================================================
// DeleteNoticeCommand - Delete an existing notice (captures for restoration)
// ============================================================================
class DeleteNoticeCommand : public ICommand
{
public:
    DeleteNoticeCommand(const QString &date, const QString &author, const QString &content)
        : m_date(date), m_author(author), m_content(content) {}

    void execute() override
    {
        ManagerNotices::deleteNotice(m_date, m_author, m_content);
    }

    void undo() override
    {
        // Re-insert the notice via CSV (to preserve the original date)
        CsvHandler::appendCsv("notices.csv", {m_date, m_author, m_content});
    }

    QString description() const override
    {
        return QString("Delete notice by %1").arg(m_author);
    }

    DataType affectedDataType() const override { return DataType::Notices; }

private:
    QString m_date, m_author, m_content;
};

// ============================================================================
// BatchCommand - Groups multiple commands as a single undo/redo step
// ============================================================================
class BatchCommand : public ICommand
{
public:
    BatchCommand(QVector<CommandPtr> commands, const QString &desc, DataType type)
        : m_commands(std::move(commands)), m_desc(desc), m_type(type) {}

    void execute() override
    {
        for (auto &cmd : m_commands)
            cmd->execute();
    }

    void undo() override
    {
        // Undo in reverse order
        for (int i = m_commands.size() - 1; i >= 0; --i)
            m_commands[i]->undo();
    }

    QString description() const override { return m_desc; }
    DataType affectedDataType() const override { return m_type; }

private:
    QVector<CommandPtr> m_commands;
    QString m_desc;
    DataType m_type;
};
