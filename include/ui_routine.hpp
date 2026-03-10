#pragma once

#include <QWidget>
#include "appmanager.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

// Routine module: manages class schedules for both students and teachers
class UIRoutine : public QObject
{
    Q_OBJECT

public:
    UIRoutine(Ui::MainWindow *ui, AcadenceManager *manager, QString role, int uid, QString name, QObject *parent = nullptr);

    void refreshRoutine();
    void refreshTeacherRoutine();

public slots:
    void onRoutineDayChanged(int index);
    void onTeacherRoutineDayChanged(int index);
    void onCancelClassClicked();
    void onRescheduleClassClicked();

private:
    Ui::MainWindow *ui;
    AcadenceManager *myManager;
    QString userRole;
    int userId;
    QString userName;
};
