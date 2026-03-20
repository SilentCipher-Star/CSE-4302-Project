#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDate>
#include <QVector>
#include <QMenu>
#include <QSet>
#include <memory>
#include <vector>
#include "appmanager.hpp"

class TeacherAttendanceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TeacherAttendanceDialog(AcadenceManager *manager, int teacherId, QWidget *parent = nullptr);
    ~TeacherAttendanceDialog();

private slots:
    void onCourseChanged(int index);
    void onCellClicked(int row, int col);
    void onAddTodayClicked();
    void onMarkAllPresentClicked();
    void onMarkAllAbsentClicked();
    void onMarkColumnPresentClicked();
    void onMarkColumnAbsentClicked();
    void onSaveClicked();
    void onExportClicked();
    void onWeekPrev();
    void onWeekNext();
    void onHeaderClicked(int logicalIndex);
    void onTableContextMenu(const QPoint &pos);

private:
    void buildTable();
    void updateStatsLabel();
    void colorCell(int row, int col, bool present);
    void recalcRowStats(int row);

    AcadenceManager *myManager;
    int teacherId;

    QComboBox *comboCourse;
    QLabel *lblStats;
    QLabel *lblWeekRange;
    QLabel *lblClassesHeld;
    QTableWidget *table;
    QPushButton *btnPrevWeek;
    QPushButton *btnNextWeek;
    QPushButton *btnAddToday;
    QPushButton *btnMarkAllPresent;
    QPushButton *btnMarkAllAbsent;
    QPushButton *btnMarkColPresent;
    QPushButton *btnMarkColAbsent;
    QPushButton *btnSave;
    QPushButton *btnExport;

    // Current state
    int currentCourseId = 0;
    int selectedDateCol = -1; // currently selected date column for column operations
    std::vector<std::unique_ptr<Student>> students;
    QVector<QString> allDates;        // all class dates sorted
    QSet<QString> currentPresenceSet; // Cached presence to avoid O(N^2) CSV calls
    int weekOffset = 0;               // 0 = latest week range shown
    int datesPerPage = 7;             // show 7 dates per page

    // Column layout:  ID=0, Name=1, then dates..., then Absent=N-2, Absent%=N-1
    int firstDateCol() const { return 2; }
    int absentCountCol() const { return 2 + visibleDateCount(); }
    int absentPctCol() const { return 3 + visibleDateCount(); }
    int visibleDateCount() const;
    QVector<QString> visibleDates() const;
};
