#pragma once

#include <QString>
#include <QMessageBox>
#include <QWidget>
#include <QRegularExpression>

/**
 * Notification Utility Class
 * Provides simplified methods for showing status messages and confirmations
 */
class Notifications
{
public:
    enum MessageType {
        Success,
        Warning,
        Error,
        Info
    };

    /**
     * Show a brief notification message (simple message box)
     */
    static void showMessage(QWidget *parent, const QString &title, const QString &message, MessageType type = Info)
    {
        QMessageBox::Icon icon = QMessageBox::Information;
        if (type == Success)
            icon = QMessageBox::Information;
        else if (type == Warning)
            icon = QMessageBox::Warning;
        else if (type == Error)
            icon = QMessageBox::Critical;

        QMessageBox::information(parent, title, message, QMessageBox::Ok);
    }

    /**
     * Show success notification
     */
    static void success(QWidget *parent, const QString &message)
    {
        showMessage(parent, "Success", message, Success);
    }

    /**
     * Show error notification
     */
    static void error(QWidget *parent, const QString &message)
    {
        showMessage(parent, "Error", message, Error);
    }

    /**
     * Show warning notification  
     */
    static void warning(QWidget *parent, const QString &message)
    {
        showMessage(parent, "Warning", message, Warning);
    }

    /**
     * Show info notification
     */
    static void info(QWidget *parent, const QString &message)
    {
        showMessage(parent, "Information", message, Info);
    }

    /**
     * Ask for confirmation before deleting
     * Returns true if user confirms, false otherwise
     */
    static bool confirmDelete(QWidget *parent, const QString &itemName = "this item")
    {
        int ret = QMessageBox::question(parent, "Confirm Deletion",
                                        "Are you sure you want to delete " + itemName + "?\n\nThis action cannot be undone.",
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No);
        return ret == QMessageBox::Yes;
    }

    /**
     * Ask for confirmation for a generic action
     */
    static bool confirm(QWidget *parent, const QString &title, const QString &message)
    {
        int ret = QMessageBox::question(parent, title, message,
                                        QMessageBox::Yes | QMessageBox::No,
                                        QMessageBox::No);
        return ret == QMessageBox::Yes;
    }

    /**
     * Validate password strength
     * Returns error message string (empty if valid)
     */
    static QString validatePassword(const QString &password)
    {
        if (password.length() < 6)
            return "Password must be at least 6 characters long.";
        if (!password.contains(QRegularExpression("[0-9]")))
            return "Password must contain at least one number.";
        if (!password.contains(QRegularExpression("[A-Za-z]")))
            return "Password must contain at least one letter.";
        return ""; // Valid
    }

    /**
     * Validate email format
     */
    static bool isValidEmail(const QString &email)
    {
        QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
        return emailRegex.match(email).hasMatch();
    }
};
