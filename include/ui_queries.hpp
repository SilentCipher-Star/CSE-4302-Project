#pragma once

#include <QWidget>
#include <QListWidgetItem>
#include "appmanager.hpp"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

// Queries module: manages Q&A system between students and teachers
class UIQueries : public QObject
{
    Q_OBJECT

public:
    UIQueries(Ui::MainWindow *ui, AcadenceManager *manager, QString role, int uid, QObject *parent = nullptr);

    void refreshQueries();

public slots:
    void onQueryActionClicked();

private:
    Ui::MainWindow *ui;
    AcadenceManager *myManager;
    QString userRole;
    int userId;
};
