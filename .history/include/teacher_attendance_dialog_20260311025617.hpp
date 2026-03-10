#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QDateEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDate>
#include <QVector>
#include <QMap>
#include "appmanager.hpp"

class TeacherAttendanceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TeacherAttendanceDialog(AcadenceManager *manager, int teacherId, QWidget *parent = nullptr);

private slots:
    void onCourseChanged(int index);
    void onWeekChanged();
    void onPrevWeek();
    void onNextWeek();
    void onCellClicked(int row, int col);
    void onMarkAllPresent();
    void onMarkAllAbsent();
    void onMarkColumnPresent();
    void onMarkColumnAbsent();
    void onAddClassDate();
    void onSaveAttendance();
    void onExportAttendance();

private:
    void buildUI();
    void refreshTable();
    void updateWeekLabel();
    void updateSummaryStats();
    void colorCell(int row, int col, bool present);
    QDate getWeekStart() const;
    QDate getWeekEnd() const;

    AcadenceManager *m_manager;
    int m_teacherId;
    QDate m_currentWeekStart;

    // UI elements
    QComboBox *m_comboCourse;
    QLabel *m_lblWeek;
    QLabel *m_lblClassesHeld;
    QLabel *m_lblStats;
    QPushButton *m_btnPrevWeek;
    QPushButton *m_btnNextWeek;
    QPushButton *m_btnMarkAllPresent;
    QPushButton *m_btnMarkAllAbsent;
    QPushButton *m_btnMarkColPresent;
    QPushButton *m_btnMarkColAbsent;
    QPushButton *m_btnAddDate;
    QPushButton *m_btnSave;
    QPushButton *m_btnExport;
    QPushButton *m_btnClose;
    QTableWidget *m_table;

    // Data cache
    struct StudentRow
    {
        int studentId;
        QString name;
    };
    QVector<StudentRow> m_students;
    QVector<QString> m_allDates;       // all dates for selected course
    QVector<QString> m_weekDates;      // dates visible in current week
    QMap<QPair<int, QString>, bool> m_attendanceCache;  // (studentId, date) -> present
    int m_selectedColumn = -1;
};
