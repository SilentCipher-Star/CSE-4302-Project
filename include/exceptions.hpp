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

    class InvalidDataException : public Exception
    {
    public:
        explicit InvalidDataException(const QString &message) : Exception(message) {}
    };

    class FileNotFoundException : public FileException
    {
    public:
        explicit FileNotFoundException(const QString &path)
            : FileException("File not found: " + path) {}
    };

    class FilePermissionException : public FileException
    {
    public:
        explicit FilePermissionException(const QString &path)
            : FileException("Permission denied: " + path) {}
    };

    class GPAOverflowException : public GradeCalculationException
    {
    public:
        explicit GPAOverflowException(const QString &message)
            : GradeCalculationException(message) {}
    };

    class InvalidGradeRangeException : public GradeCalculationException
    {
    private:
        double m_value;
        double m_max;
    public:
        InvalidGradeRangeException(double value, double max)
            : GradeCalculationException(
                  QString("Grade %1 exceeds maximum %2").arg(value).arg(max)),
              m_value(value), m_max(max) {}
        double getValue() const { return m_value; }
        double getMax() const { return m_max; }
    };

    class MissingFieldException : public InvalidDataException
    {
    public:
        explicit MissingFieldException(const QString &fieldName)
            : InvalidDataException("Required field missing: " + fieldName) {}
    };

    class InvalidDateFormatException : public InvalidDataException
    {
    public:
        explicit InvalidDateFormatException(const QString &value)
            : InvalidDataException("Invalid date format: " + value) {}
    };

    class EnrollmentException : public Exception
    {
    public:
        explicit EnrollmentException(const QString &message) : Exception(message) {}
    };

    class CourseCapacityException : public EnrollmentException
    {
    public:
        explicit CourseCapacityException(const QString &courseName)
            : EnrollmentException("Course is full: " + courseName) {}
    };

}

#endif // EXCEPTIONS_HPP