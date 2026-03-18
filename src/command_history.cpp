#include "../include/command_history.hpp"

CommandHistory::CommandHistory(int maxSize)
    : m_maxSize(maxSize)
{
}

void CommandHistory::push(CommandPtr cmd)
{
    m_undoStack.push(cmd);
    m_redoStack.clear();

    // Trim if exceeding max size
    while (m_undoStack.size() > m_maxSize)
        m_undoStack.removeFirst();
}

CommandPtr CommandHistory::undo()
{
    if (m_undoStack.isEmpty())
        return nullptr;

    CommandPtr cmd = m_undoStack.pop();
    cmd->undo();
    m_redoStack.push(cmd);
    return cmd;
}

CommandPtr CommandHistory::redo()
{
    if (m_redoStack.isEmpty())
        return nullptr;

    CommandPtr cmd = m_redoStack.pop();
    cmd->execute();
    m_undoStack.push(cmd);
    return cmd;
}

bool CommandHistory::canUndo() const
{
    return !m_undoStack.isEmpty();
}

bool CommandHistory::canRedo() const
{
    return !m_redoStack.isEmpty();
}

QString CommandHistory::undoDescription() const
{
    if (m_undoStack.isEmpty())
        return QString();
    return m_undoStack.top()->description();
}

QString CommandHistory::redoDescription() const
{
    if (m_redoStack.isEmpty())
        return QString();
    return m_redoStack.top()->description();
}

void CommandHistory::clear()
{
    m_undoStack.clear();
    m_redoStack.clear();
}
