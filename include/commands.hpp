#pragma once

#include "icommand.hpp"
#include "appmanager.hpp"
#include "manager_academics.hpp"
#include "manager_productivity.hpp"
#include "manager_community.hpp"
#include "csvhandler.hpp"
#include <QVector>
#include <algorithm>

/**
 * Concrete undoable actions wrapping calls to managers
 */

/**
 * Updates a single student's presence status
 */
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

/**
 * Overwrites score for specific student and test
 */
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
            // Delete generated grade row if original status was empty
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

/**
 * Processes sequential group attendance changes
 */
class BatchMarkAttendanceCommand : public ICommand
{
public:
    BatchMarkAttendanceCommand(int courseId, const QVector<ManagerAcademics::AttendanceUpdate> &newUpdates)
        : m_courseId(courseId), m_newUpdates(newUpdates)
    {
        for (const auto &u : newUpdates)
        {
            ManagerAcademics::AttendanceUpdate oldU;
            oldU.studentId = u.studentId;
            oldU.date = u.date;
            oldU.present = ManagerAcademics::isPresent(courseId, u.studentId, u.date);
            m_oldUpdates.append(oldU);
        }
    }

    void execute() override
    {
        ManagerAcademics::markAttendanceBatch(m_courseId, m_newUpdates);
    }

    void undo() override
    {
        ManagerAcademics::markAttendanceBatch(m_courseId, m_oldUpdates);
    }

    QString description() const override { return "Batch update attendance"; }
    DataType affectedDataType() const override { return DataType::Academics; }

private:
    int m_courseId;
    QVector<ManagerAcademics::AttendanceUpdate> m_newUpdates;
    QVector<ManagerAcademics::AttendanceUpdate> m_oldUpdates;
};

/**
 * Saves test scores across a class sequentially
 */
class BatchAddGradeCommand : public ICommand
{
public:
    BatchAddGradeCommand(int assessmentId, const QVector<ManagerAcademics::GradeUpdate> &newUpdates)
        : m_assessmentId(assessmentId), m_newUpdates(newUpdates)
    {
        for (const auto &u : newUpdates)
        {
            ManagerAcademics::GradeUpdate oldU;
            oldU.studentId = u.studentId;
            oldU.marks = ManagerAcademics::getGrade(u.studentId, assessmentId);
            m_oldUpdates.append(oldU);
        }
    }

    void execute() override
    {
        ManagerAcademics::addGradeBatch(m_assessmentId, m_newUpdates);
    }

    void undo() override
    {
        ManagerAcademics::addGradeBatch(m_assessmentId, m_oldUpdates);
    }

    QString description() const override { return "Batch update grades"; }
    DataType affectedDataType() const override { return DataType::Academics; }

private:
    int m_assessmentId;
    QVector<ManagerAcademics::GradeUpdate> m_newUpdates;
    QVector<ManagerAcademics::GradeUpdate> m_oldUpdates;
};

/**
 * Inserts a habit or study target item
 */
class AddTaskCommand : public ICommand
{
public:
    AddTaskCommand(int userId, const QString &desc)
        : m_userId(userId), m_desc(desc), m_assignedId(-1) {}

    void execute() override
    {
        // Calculate resulting row index before assignment
        QVector<QStringList> data = CsvHandler::readCsv("tasks.csv");
        int maxId = 0;
        for (const auto &row : data)
            if (!row.isEmpty())
                maxId = std::max(maxId, row[0].toInt());
        m_assignedId = maxId + 1;

        ManagerProductivity::addTask(m_userId, m_desc);
    }

    void undo() override
    {
        if (m_assignedId > 0)
            ManagerProductivity::deleteTask(m_assignedId);
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

/**
 * Wipes out an entry from task list and stores layout for reversal
 */
class DeleteTaskCommand : public ICommand
{
public:
    DeleteTaskCommand(int taskId, int userId, const QString &desc, bool wasCompleted)
        : m_taskId(taskId), m_userId(userId), m_desc(desc), m_wasCompleted(wasCompleted) {}

    void execute() override
    {
        ManagerProductivity::deleteTask(m_taskId);
    }

    void undo() override
    {
        // Replace item with identical column properties
        CsvHandler::appendCsv("tasks.csv", {QString::number(m_taskId),
                                            QString::number(m_userId),
                                            m_desc,
                                            m_wasCompleted ? "1" : "0"});
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

/**
 * Switches activity toggle values dynamically
 */
class CompleteTaskCommand : public ICommand
{
public:
    CompleteTaskCommand(int taskId, bool oldStatus, bool newStatus)
        : m_taskId(taskId), m_oldStatus(oldStatus), m_newStatus(newStatus) {}

    void execute() override
    {
        ManagerProductivity::completeTask(m_taskId, m_newStatus);
    }

    void undo() override
    {
        ManagerProductivity::completeTask(m_taskId, m_oldStatus);
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

/**
 * Wipes finished targets while backing them up internally
 */
class DeleteCompletedTasksCommand : public ICommand
{
public:
    DeleteCompletedTasksCommand(int userId)
        : m_userId(userId)
    {
        // Pull snapshot of completed entries only
        QVector<Task> tasks = ManagerProductivity::getTasks(userId);
        for (const auto &t : tasks)
        {
            if (t.getIsCompleted())
                m_deletedTasks.append({t.getId(), t.getDescription()});
        }
    }

    void execute() override
    {
        ManagerProductivity::deleteCompletedTasks(m_userId);
    }

    void undo() override
    {
        // Cycle via history and recover item structures
        for (const auto &t : m_deletedTasks)
        {
            CsvHandler::appendCsv("tasks.csv", {QString::number(t.first),
                                                QString::number(m_userId),
                                                t.second,
                                                "1"});
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

/**
 * Appends community board item
 */
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
        ManagerCommunity::addNotice(m_content, m_author);
    }

    void undo() override
    {
        ManagerCommunity::deleteNotice(m_date, m_author, m_content);
    }

    QString description() const override
    {
        return QString("Post notice by %1").arg(m_author);
    }

    DataType affectedDataType() const override { return DataType::Notices; }

private:
    QString m_content, m_author, m_date;
};

/**
 * Removes community item saving a ghost copy for rollback
 */
class DeleteNoticeCommand : public ICommand
{
public:
    DeleteNoticeCommand(const QString &date, const QString &author, const QString &content)
        : m_date(date), m_author(author), m_content(content) {}

    void execute() override
    {
        ManagerCommunity::deleteNotice(m_date, m_author, m_content);
    }

    void undo() override
    {
        // Append identical details directly over CSV handler logic
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

/**
 * Bundles unrelated operations underneath a single action log
 */
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
        // Ensure trailing operations are canceled first to avoid cascading faults
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
