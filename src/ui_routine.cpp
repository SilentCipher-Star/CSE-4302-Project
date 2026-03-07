#include "../include/ui_routine.hpp"
#include "../include/utils.hpp"
#include "../include/student.hpp"
#include "../include/course.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>
#include <memory>

UIRoutine::UIRoutine(Ui::MainWindow *ui, AcadenceManager *manager, QString role, int uid, QString name, QObject *parent)
    : QObject(parent), ui(ui), myManager(manager), userRole(role), userId(uid), userName(name)
{
}

void UIRoutine::refreshRoutine()
{
    ui->tableRoutine->setRowCount(0);
    QString day = ui->comboRoutineDay->currentText();

    int semester = -1;
    if (userRole == Constants::Role::Student)
    {
        std::unique_ptr<Student> s(myManager->getStudent(userId));
        if (s)
        {
            semester = s->getSemester();
        }
    }

    QVector<RoutineSession> items = myManager->getEffectiveRoutine(Utils::getDateForDay(day), semester);

    ui->tableRoutine->setColumnCount(4);
    QStringList headers = {"Time", "Course", "Room", "Instructor"};
    ui->tableRoutine->setHorizontalHeaderLabels(headers);

    for (const auto &i : items)
    {
        int row = ui->tableRoutine->rowCount();
        ui->tableRoutine->insertRow(row);
        ui->tableRoutine->setItem(row, 0, new QTableWidgetItem(i.getStartTime() + " - " + i.getEndTime()));
        ui->tableRoutine->setItem(row, 1, new QTableWidgetItem(i.getCourseCode() + ": " + i.getCourseName()));
        ui->tableRoutine->setItem(row, 2, new QTableWidgetItem(i.getRoom()));
        ui->tableRoutine->setItem(row, 3, new QTableWidgetItem(i.getInstructor()));
    }
    Utils::adjustColumnWidths(ui->tableRoutine);
}

void UIRoutine::onRoutineDayChanged(int index)
{
    refreshRoutine();
}

void UIRoutine::refreshTeacherRoutine()
{
    ui->tableTeacherRoutine->setRowCount(0);
    QString day = ui->comboRoutineDayInput->currentText();
    QVector<RoutineSession> items = myManager->getEffectiveRoutine(Utils::getDateForDay(day));

    if (userRole == Constants::Role::Teacher)
    {
        QVector<Course *> courses = myManager->getTeacherCourses(userId);
        QStringList myCodes;
        for (Course *c : courses)
        {
            myCodes << c->getCode();
        }
        qDeleteAll(courses);

        QVector<RoutineSession> filtered;
        for (const auto &item : items)
        {
            if (myCodes.contains(item.getCourseCode()))
            {
                filtered.append(item);
            }
        }
        items = filtered;
    }

    for (const auto &i : items)
    {
        int row = ui->tableTeacherRoutine->rowCount();
        ui->tableTeacherRoutine->insertRow(row);
        ui->tableTeacherRoutine->setItem(row, 0, new QTableWidgetItem(i.getStartTime() + " - " + i.getEndTime()));
        ui->tableTeacherRoutine->setItem(row, 1, new QTableWidgetItem(i.getCourseCode() + ": " + i.getCourseName()));
        ui->tableTeacherRoutine->setItem(row, 2, new QTableWidgetItem(i.getRoom()));
        ui->tableTeacherRoutine->setItem(row, 3, new QTableWidgetItem(QString::number(i.getSemester())));

        ui->tableTeacherRoutine->item(row, 0)->setData(Qt::UserRole, i.getStartTime());
        ui->tableTeacherRoutine->item(row, 1)->setData(Qt::UserRole, i.getCourseCode());
    }
    Utils::adjustColumnWidths(ui->tableTeacherRoutine);
}

void UIRoutine::onTeacherRoutineDayChanged(int index)
{
    refreshTeacherRoutine();
}

void UIRoutine::onCancelClassClicked()
{
    int row = ui->tableTeacherRoutine->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(nullptr, "Selection", "Please select a class to cancel.");
        return;
    }

    QString day = ui->comboRoutineDayInput->currentText();
    QDate date = Utils::getDateForDay(day);
    QString startTimeStr = ui->tableTeacherRoutine->item(row, 0)->data(Qt::UserRole).toString();
    int serial = 0;
    if (startTimeStr == "09:00")
        serial = 1;
    else if (startTimeStr == "10:00")
        serial = 2;
    else if (startTimeStr == "11:00")
        serial = 3;
    else if (startTimeStr == "12:00")
        serial = 4;
    else if (startTimeStr == "14:00")
        serial = 5;

    QString code = ui->tableTeacherRoutine->item(row, 1)->data(Qt::UserRole).toString();
    int semester = ui->tableTeacherRoutine->item(row, 3)->text().toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(nullptr, "Confirm Cancellation",
                                  "Are you sure you want to cancel the class on " + date.toString("yyyy-MM-dd") + "?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        RoutineAdjustment adj;
        adj.originalDate = date.toString(Qt::ISODate);
        adj.originalSerial = serial;
        adj.type = "CANCEL";
        adj.courseCode = code;
        adj.semester = semester;

        myManager->addRoutineAdjustment(adj);
        QMessageBox::information(nullptr, "Success", "Class cancelled.");
    }
}

void UIRoutine::onRescheduleClassClicked()
{
    int row = ui->tableTeacherRoutine->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(nullptr, "Selection", "Please select a class to exchange/reschedule.");
        return;
    }

    QString currentDayStr = ui->comboRoutineDayInput->currentText();
    QDate originDate = Utils::getDateForDay(currentDayStr);
    QString originStartStr = ui->tableTeacherRoutine->item(row, 0)->data(Qt::UserRole).toString();
    int originSerial = 0;
    if (originStartStr == "09:00")
        originSerial = 1;
    else if (originStartStr == "10:00")
        originSerial = 2;
    else if (originStartStr == "11:00")
        originSerial = 3;
    else if (originStartStr == "12:00")
        originSerial = 4;
    else if (originStartStr == "14:00")
        originSerial = 5;

    QString originCode = ui->tableTeacherRoutine->item(row, 1)->data(Qt::UserRole).toString();
    QString originRoom = ui->tableTeacherRoutine->item(row, 2)->text();
    int semester = ui->tableTeacherRoutine->item(row, 3)->text().toInt();

    QVector<RescheduleOption> optionsList = myManager->getRescheduleOptions(originDate, originSerial, semester, originCode, originRoom, userName);

    QStringList options;
    for (const auto &opt : optionsList)
    {
        options << opt.displayText;
    }

    bool ok;
    QString choice = QInputDialog::getItem(nullptr, "Select New Slot", "Available Options (Next 2 Weeks):", options, 0, false, &ok);
    if (ok && !choice.isEmpty())
    {
        int idx = options.indexOf(choice);
        if (idx >= 0 && idx < optionsList.size())
        {
            myManager->addRoutineAdjustment(optionsList[idx].adjustment);

            if (!optionsList[idx].secondaryAdjustment.courseCode.isEmpty())
            {
                myManager->addRoutineAdjustment(optionsList[idx].secondaryAdjustment);
            }

            QMessageBox::information(nullptr, "Success", "Class rescheduled/exchanged successfully.");
        }
    }
}
