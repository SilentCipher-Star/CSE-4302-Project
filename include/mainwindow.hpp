#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidgetItem>
#include "academicmanager.hpp"
#include "timer.hpp"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString role, int userId, QString userName, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_btnAddRow_clicked();
    void on_btnDeleteRow_clicked();
    void on_tableComboBox_currentTextChanged(const QString &arg1);
    void on_searchLineEdit_textChanged(const QString &arg1);

    void on_addTaskButton_clicked();
    void on_completeTaskButton_clicked();
    void on_btnAddHabit_clicked();
    void on_btnPerformHabit_clicked();
    void on_btnDeleteHabit_clicked();
    void on_addNoticeButton_clicked();
    void on_logoutButton_clicked();

    void on_btnTimerStart_clicked();
    void on_btnTimerPause_clicked();
    void on_btnTimerStop_clicked();

    void on_btnWorkoutStart_clicked();
    void on_btnWorkoutPause_clicked();
    void on_btnWorkoutStop_clicked();

    void on_comboRoutineDay_currentIndexChanged(int index);
    void on_comboRoutineDayInput_currentIndexChanged(int index);
    void on_btnAddRoutine_Teacher_clicked();
    void on_btnCreateAssessment_clicked();
    void on_comboTeacherAssessment_currentIndexChanged(int index);
    void on_btnSaveGrades_clicked();
    void on_comboAttendanceCourse_currentIndexChanged(int index);
    void on_btnAddClassDate_clicked();
    void on_btnSaveAttendance_clicked();
    void on_btnQueryAction_clicked();

    void on_chkFajr_toggled(bool checked);
    void on_chkDhuhr_toggled(bool checked);
    void on_chkAsr_toggled(bool checked);
    void on_chkMaghrib_toggled(bool checked);
    void on_chkIsha_toggled(bool checked);

    void onChangePasswordClicked();

private:
    Ui::MainWindow *ui;
    AcadenceManager myManager;

    QVector<Student *> currentStudentList;
    QVector<Teacher *> currentTeacherList;
    QVector<Course *> currentCourseList;
    QVector<Habit *> currentHabitList;

    Timer *m_focusTimer;
    Timer *m_workoutTimer;
    DurationHabit *activeTimerHabit;

    QStandardItemModel *adminModel;
    QSortFilterProxyModel *adminProxyModel;
    CsvDelegate *csvDelegate;

    QString userRole;
    int userId;
    QString userName;

    void setupTables();
    void setupTimers();
    void setupConnections();

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

#endif // MAINWINDOW_H