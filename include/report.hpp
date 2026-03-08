#pragma once
#include <QString>
#include <QVector>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include "appmanager.hpp"

namespace Acadence {

class IReport
{
public:
    virtual ~IReport() = default;

    void setData(const QString &studentName,
                 const QVector<AttendanceRecord> &attendance,
                 const QVector<Assessment> &assessments)
    {
        m_studentName  = studentName;
        m_attendance   = attendance;
        m_assessments  = assessments;
        m_generatedOn  = QDate::currentDate();
    }

    void generate(const QString &path)
    {
        if (!openFile(path)) return;
        writeHeader();
        writeBody();
        writeFooter();
        closeFile();
    }

    virtual QString formatName() const = 0;
    virtual QString fileExtension() const = 0;

protected:
    virtual bool openFile(const QString &path) = 0;
    virtual void writeHeader() = 0;
    virtual void writeBody() = 0;
    virtual void writeFooter() = 0;
    virtual void closeFile() = 0;

    QString m_studentName;
    QVector<AttendanceRecord> m_attendance;
    QVector<Assessment> m_assessments;
    QDate m_generatedOn;
};

class CSVReport : public IReport
{
public:
    QString formatName() const override { return "CSV"; }
    QString fileExtension() const override { return "csv"; }

protected:
    QFile m_file;
    QTextStream m_out;

    bool openFile(const QString &path) override
    {
        m_file.setFileName(path);
        if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        m_out.setDevice(&m_file);
        return true;
    }

    void writeHeader() override
    {
        m_out << "Academic Report - " << m_studentName << "\n";
        m_out << "Generated," << m_generatedOn.toString("yyyy-MM-dd") << "\n\n";
        m_out << "Course,Attendance %,Score,Status\n";
    }

    void writeBody() override
    {
        for (const auto &rec : m_attendance)
        {
            double pct = (rec.getTotalClasses() > 0)
                             ? (double)rec.getAttendedClasses() / rec.getTotalClasses() * 100.0
                             : 0.0;
            QString status = (pct >= 75.0) ? "Good" : (pct >= 60.0) ? "At Risk" : "Critical";
            QString score = QString::number(rec.getTotalMarksObtained()) + "/" +
                            QString::number(rec.getTotalMaxMarks());
            m_out << rec.getCourseName() << "," << QString::number(pct, 'f', 1) << "%,"
                  << score << "," << status << "\n";
        }

        m_out << "\nAssessments\nDate,Course,Title,Type,Countdown\n";
        QDate today = QDate::currentDate();
        for (const auto &a : m_assessments)
        {
            QDate d = QDate::fromString(a.getDate(), "yyyy-MM-dd");
            QString countdown;
            if (d.isValid())
            {
                int dl = today.daysTo(d);
                countdown = (dl >= 0) ? QString("%1 day(s) left").arg(dl) : "Past";
            }
            m_out << a.getDate() << "," << a.getCourseName() << ","
                  << a.getTitle() << "," << a.getType() << "," << countdown << "\n";
        }
    }

    void writeFooter() override
    {
        m_out << "\n--- End of Report ---\n";
    }

    void closeFile() override { m_file.close(); }
};

class TextReport : public IReport
{
public:
    QString formatName() const override { return "Text"; }
    QString fileExtension() const override { return "txt"; }

protected:
    QFile m_file;
    QTextStream m_out;

    bool openFile(const QString &path) override
    {
        m_file.setFileName(path);
        if (!m_file.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;
        m_out.setDevice(&m_file);
        return true;
    }

    void writeHeader() override
    {
        m_out << QString(60, '=') << "\n";
        m_out << "         ACADENCE - ACADEMIC REPORT\n";
        m_out << QString(60, '=') << "\n";
        m_out << "Student : " << m_studentName << "\n";
        m_out << "Date    : " << m_generatedOn.toString("dd MMM yyyy") << "\n";
        m_out << QString(60, '-') << "\n\n";
    }

    void writeBody() override
    {
        m_out << "ATTENDANCE & GRADES\n";
        m_out << QString(60, '-') << "\n";
        for (const auto &rec : m_attendance)
        {
            double pct = (rec.getTotalClasses() > 0)
                             ? (double)rec.getAttendedClasses() / rec.getTotalClasses() * 100.0
                             : 0.0;
            QString status = (pct >= 75.0) ? "Good" : (pct >= 60.0) ? "At Risk" : "Critical";
            QString score = QString::number(rec.getTotalMarksObtained()) + "/" +
                            QString::number(rec.getTotalMaxMarks());
            m_out << rec.getCourseName().leftJustified(28)
                  << (QString::number(pct, 'f', 1) + "%").leftJustified(10)
                  << score.leftJustified(14)
                  << status << "\n";
        }

        m_out << "\nASSESSMENTS\n" << QString(60, '-') << "\n";
        QDate today = QDate::currentDate();
        for (const auto &a : m_assessments)
        {
            QDate d = QDate::fromString(a.getDate(), "yyyy-MM-dd");
            int dl = today.daysTo(d);
            QString countdown = d.isValid() ? ((dl >= 0) ? QString("%1d left").arg(dl) : "Past") : "";
            m_out << a.getDate().leftJustified(13)
                  << a.getCourseName().left(18).leftJustified(20)
                  << a.getTitle().left(24).leftJustified(26)
                  << a.getType().leftJustified(12)
                  << countdown << "\n";
        }
    }

    void writeFooter() override
    {
        m_out << "\n" << QString(60, '=') << "\n";
        m_out << "               End of Report\n";
        m_out << QString(60, '=') << "\n";
    }

    void closeFile() override { m_file.close(); }
};

} // namespace Acadence
