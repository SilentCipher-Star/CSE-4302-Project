#pragma once

#include <QWidget>
#include "appmanager.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

// Academics module: manages academic tools for teachers (assessments, grading, attendance)
// and academic view for students
class UIAcademics : public QObject
{
    Q_OBJECT

public:
    UIAcademics(Ui::MainWindow *ui, AcadenceManager *manager, QString role, int uid, QObject *parent = nullptr);

    void refreshAcademics();
    void refreshTeacherTools();
    void refreshTeacherGrades();
    void refreshTeacherAttendance();

public slots:
    void onCreateAssessmentClicked();
    void onTeacherAssessmentChanged(int index);
    void onSaveGradesClicked();
    void onAttendanceCourseChanged(int index);
    void onAddClassDateClicked();
    void onSaveAttendanceClicked();

private:
    Ui::MainWindow *ui;
    AcadenceManager *myManager;
    QString userRole;
    int userId;
};
