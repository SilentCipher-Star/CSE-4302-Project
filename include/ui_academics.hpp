#pragma once

#include <QWidget>
#include <QPushButton>
#include "appmanager.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class UIAcademics : public QObject
{
    Q_OBJECT

public:
    UIAcademics(Ui::MainWindow *ui, AcadenceManager *manager, QString role, int uid, QObject *parent = nullptr);

    void refreshAcademics();
    void refreshTeacherTools();
    void refreshTeacherGrades();
    void refreshTeacherAttendance();

    QPushButton *getCheckWarningsButton() const { return btnCheckWarnings; }

public slots:
    void onCreateAssessmentClicked();
    void onTeacherAssessmentChanged(int index);
    void onSaveGradesClicked();
    void onAttendanceCourseChanged(int index);
    void onAddClassDateClicked();
    void onSaveAttendanceClicked();
    void onCheckAttendanceWarningsClicked();
    void onGPACalculatorClicked();
    void onExportReportClicked();
    void onAttendanceSimulatorClicked();

private:
    Ui::MainWindow *ui;
    AcadenceManager *myManager;
    QString userRole;
    int userId;
    QPushButton *btnCheckWarnings;
    QPushButton *btnGPACalc   = nullptr;
    QPushButton *btnExport    = nullptr;
    QPushButton *btnAttendSim = nullptr;
};
