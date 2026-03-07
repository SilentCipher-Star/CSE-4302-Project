#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include <exception>
#include <string>
#include <QString>

namespace Acadence
{

    class Exception : public std::exception
    {
    protected:
        std::string msg;

    public:
        explicit Exception(const QString &message) : msg(message.toStdString()) {}
        const char *what() const noexcept override { return msg.c_str(); }
    };

    class FileException : public Exception
    {
    public:
        explicit FileException(const QString &message) : Exception(message) {}
    };

    class DuplicateRecordException : public Exception
    {
    public:
        explicit DuplicateRecordException(const QString &message) : Exception(message) {}
    };

    class InvalidCredentialsException : public Exception
    {
    public:
        explicit InvalidCredentialsException(const QString &message) : Exception(message) {}
    };

    class InsufficientAttendanceException : public Exception
    {
    public:
        explicit InsufficientAttendanceException(const QString &message) : Exception(message) {}
    };

    class GradeCalculationException : public Exception
    {
    public:
        explicit GradeCalculationException(const QString &message) : Exception(message) {}
    };

    class DatabaseException : public Exception
    {
    public:
        explicit DatabaseException(const QString &message) : Exception(message) {}
    };

}

#endif // EXCEPTIONS_HPP