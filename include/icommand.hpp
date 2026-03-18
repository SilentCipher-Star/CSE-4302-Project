#pragma once

#include <QString>
#include <memory>

// Forward declaration
enum class DataType;

/**
 * Command Pattern - ICommand Interface
 *
 * Each command encapsulates a single data-modifying operation
 * with the ability to execute, undo, and describe itself.
 * This enables a full Undo/Redo system across the application.
 */
class ICommand
{
public:
    virtual ~ICommand() = default;

    /// Perform the action
    virtual void execute() = 0;

    /// Reverse the action
    virtual void undo() = 0;

    /// Human-readable description (shown in Undo/Redo tooltips)
    virtual QString description() const = 0;

    /// Which DataType this command affects (for observer notifications)
    virtual DataType affectedDataType() const = 0;
};

using CommandPtr = std::shared_ptr<ICommand>;
