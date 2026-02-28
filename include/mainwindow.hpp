#pragma once

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QPushButton>
#include "academicmanager.hpp"
#include "timer.hpp"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

// Forward declaration of MainWindow namespace to keep compilation fast
QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

// Class for handling the creation of editors like SpinBox, DateEdit, ComboBox
class CsvDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CsvDelegate(QObject *parent = nullptr);
    QString currentTable;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};

// The main application window
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString role, int userId, QString userName, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Admin Panel Slots
    void on_btnAddRow_clicked();
    void on_btnDeleteRow_clicked();
    void on_tableComboBox_currentTextChanged(const QString &arg1);
    void on_searchLineEdit_textChanged(const QString &arg1);

    // Study Planner Slots
    void on_addTaskButton_clicked();
    void on_btnDeleteTask_clicked();
    void on_taskItemChanged(QListWidgetItem *item);

    // Habit Tracker Slots
    void on_btnAddHabit_clicked();
    void on_btnPerformHabit_clicked();
    void on_btnDeleteHabit_clicked();

    // Dashboard Slots
    void on_addNoticeButton_clicked();
    void on_logoutButton_clicked();

    // Focus Timer Slots
    void on_btnTimerStart_clicked();
    void on_btnTimerPause_clicked();
    void on_btnTimerStop_clicked();

    // Workout/Habit Timer Slots
    void on_btnWorkoutStart_clicked();
    void on_btnWorkoutPause_clicked();
    void on_btnWorkoutStop_clicked();

    // Routine/Schedule Slots
    void on_comboRoutineDay_currentIndexChanged(int index);
    void on_comboRoutineDayInput_currentIndexChanged(int index);
    void on_btnAddRoutine_Teacher_clicked();

    // Teacher Academic Tools Slots
    void on_btnCreateAssessment_clicked();
    void on_comboTeacherAssessment_currentIndexChanged(int index);
    void on_btnSaveGrades_clicked();
    void on_comboAttendanceCourse_currentIndexChanged(int index);
    void on_btnAddClassDate_clicked();
    void on_btnSaveAttendance_clicked();

    // Q&A Slots
    void on_btnQueryAction_clicked();

    // Prayer Tracker Slots
    void on_chkFajr_toggled(bool checked);
    void on_chkDhuhr_toggled(bool checked);
    void on_chkAsr_toggled(bool checked);
    void on_chkMaghrib_toggled(bool checked);
    void on_chkIsha_toggled(bool checked);

    // Profile Slots
    void onChangePasswordClicked();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    AcadenceManager myManager;

    // Caches for memory management
    QVector<Student *> currentStudentList;
    QVector<Teacher *> currentTeacherList;
    QVector<Course *> currentCourseList;
    QVector<Habit *> currentHabitList;

    // Timer objects
    Timer *m_focusTimer;
    Timer *m_workoutTimer;
    DurationHabit *activeTimerHabit;
    QPushButton *btnDeleteTask;

    // Admin View Models
    QStandardItemModel *adminModel;
    QSortFilterProxyModel *adminProxyModel;
    CsvDelegate *csvDelegate;

    // User Session Data
    QString userRole;
    int userId;
    QString userName;

    // Initialization Helpers
    void setupTables();
    void setupTimers();
    void setupConnections();

    // Data Refresh Helpers
    void refreshDashboard();
    void refreshPlanner();
    void refreshHabits();
    void refreshRoutine();
    void refreshTeacherRoutine();
    void refreshAcademics();
    void refreshTeacherTools();
    void refreshTeacherGrades();
    void refreshTeacherAttendance();
    void refreshQueries();
};
