#pragma once

#include "icommand.hpp"
#include <QStack>

/**
 * Command Pattern - CommandHistory Manager
 *
 * Maintains an undo stack and a redo stack.
 * When a new command is executed, the redo stack is cleared
 * (standard undo/redo semantics). The undo stack is trimmed
 * if it exceeds the configured maximum size.
 */
class CommandHistory
{
public:
    explicit CommandHistory(int maxSize = 50);

    /// Push a command after it has been executed
    void push(CommandPtr cmd);

    /// Undo the most recent command; returns it (or nullptr if empty)
    CommandPtr undo();

    /// Redo the most recently undone command; returns it (or nullptr if empty)
    CommandPtr redo();

    bool canUndo() const;
    bool canRedo() const;

    /// Description of the command at the top of the undo stack
    QString undoDescription() const;

    /// Description of the command at the top of the redo stack
    QString redoDescription() const;

    /// Clear all history
    void clear();

private:
    QStack<CommandPtr> m_undoStack;
    QStack<CommandPtr> m_redoStack;
    int m_maxSize;
};
