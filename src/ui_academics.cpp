#include "../include/ui_academics.hpp"
#include "../include/utils.hpp"
#include "../include/course.hpp"
#include "../include/student.hpp"
#include "../include/notifications.hpp"
#include "../include/exceptions.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>
#include <memory>

UIAcademics::UIAcademics(Ui::MainWindow *ui, AcadenceManager *manager, QString role, int uid, QObject *parent)
    : QObject(parent), ui(ui), myManager(manager), userRole(role), userId(uid), btnCheckWarnings(nullptr)
{
    if (role == "Teacher")
    {
        btnCheckWarnings = new QPushButton("Check Attendance Warnings");
        btnCheckWarnings->setMaximumWidth(250);
        ui->hbox_attendance_controls->addWidget(btnCheckWarnings);
        connect(btnCheckWarnings, &QPushButton::clicked, this, &UIAcademics::onCheckAttendanceWarningsClicked);
    }
}

void UIAcademics::refreshAcademics()
{
    ui->listAssessments->clear();
    QVector<Assessment> assessments = myManager->getStudentAssessments(userId);
    for (const auto &a : assessments)
    {
        ui->listAssessments->addItem(a.getDate() + " - " + a.getCourseName() + ": " + a.getTitle() + " (" + a.getType() + ")");
    }

    ui->tableAcademics->setRowCount(0);
    QVector<AttendanceRecord> att = myManager->getStudentAttendance(userId);

    double overallPct = myManager->getOverallAttendancePercentage(userId);
    QString overallColor;
    if (overallPct >= 85.0)
        overallColor = "green";
    else if (overallPct >= 60.0)
        overallColor = "orange";
    else
        overallColor = "red";

    ui->lbl_attendance->setText(
        QString("Attendance & Grades | Overall Attendance: <span style='color:%1; font-weight:bold;'>%2%</span>")
            .arg(overallColor)
            .arg(QString::number(overallPct, 'f', 1)));

    bool hasCritical = false;
    QString criticalCourses;

    for (int i = 0; i < att.size(); ++i)
    {
        ui->tableAcademics->insertRow(i);
        ui->tableAcademics->setItem(i, 0, new QTableWidgetItem(att[i].getCourseName()));

        double pct = (att[i].getTotalClasses() > 0) ? (double)att[i].getAttendedClasses() / att[i].getTotalClasses() * 100.0 : 0.0;

        QTableWidgetItem *pctItem = new QTableWidgetItem(QString::number(pct, 'f', 1) + "%");
        QString status;
        QColor rowColor;

        if (pct >= 85.0)
        {
            status = "Good";
            rowColor = QColor(34, 139, 34);
        }
        else if (pct >= 60.0)
        {
            status = "At Risk";
            rowColor = QColor(255, 165, 0);
        }
        else
        {
            status = "Critical";
            rowColor = QColor(220, 20, 60);
            hasCritical = true;
            if (!criticalCourses.isEmpty())
                criticalCourses += ", ";
            criticalCourses += att[i].getCourseName();
        }

        pctItem->setForeground(rowColor);
        ui->tableAcademics->setItem(i, 1, pctItem);

        QString scoreStr = QString::number(att[i].getTotalMarksObtained()) + " / " + QString::number(att[i].getTotalMaxMarks());
        ui->tableAcademics->setItem(i, 2, new QTableWidgetItem(scoreStr));

        QTableWidgetItem *statusItem = new QTableWidgetItem(status);
        statusItem->setForeground(rowColor);
        ui->tableAcademics->setItem(i, 3, statusItem);
    }
    Utils::adjustColumnWidths(ui->tableAcademics);

    if (hasCritical)
    {
        try
        {
            throw Acadence::InsufficientAttendanceException(
                "Your attendance is critically low in: " + criticalCourses +
                ". You may be barred from exams.");
        }
        catch (const Acadence::InsufficientAttendanceException &e)
        {
            Notifications::warning(nullptr, QString::fromStdString(e.what()));
        }
    }
}

void UIAcademics::refreshTeacherTools()
{
    ui->comboTeacherCourse->clear();
    ui->comboAttendanceCourse->clear();
    QVector<Course *> courses = myManager->getTeacherCourses(userId);
    for (auto c : courses)
    {
        ui->comboTeacherCourse->addItem(c->getName(), c->getId());
        ui->comboAttendanceCourse->addItem(c->getCode() + " - " + c->getName(), c->getId());
    }
    qDeleteAll(courses);

    ui->comboTeacherAssessment->clear();
    QVector<Assessment> assessments = myManager->getTeacherAssessments(userId);
    for (const auto &a : assessments)
    {
        ui->comboTeacherAssessment->addItem(a.getCourseName() + " - " + a.getTitle(), a.getId());
    }

    refreshTeacherGrades();
    refreshTeacherAttendance();
}

void UIAcademics::onCreateAssessmentClicked()
{
    int courseId = ui->comboTeacherCourse->currentData().toInt();
    QString title = ui->editAssessmentTitle->text();
    if (title.isEmpty())
        return;

    myManager->addAssessment(courseId, title, ui->comboAssessmentType->currentText(),
                             QDate::currentDate().toString("yyyy-MM-dd"), ui->spinMaxMarks->value());
    ui->editAssessmentTitle->clear();
    QMessageBox::information(nullptr, "Success", "Assessment Created");
}

void UIAcademics::onTeacherAssessmentChanged(int index)
{
    refreshTeacherGrades();
}

void UIAcademics::refreshTeacherGrades()
{
    ui->tableGrading->setRowCount(0);

    int assessmentId = ui->comboTeacherAssessment->currentData().toInt();

    QVector<Assessment> allAssessments = myManager->getAssessments();
    int courseId = -1;
    for (const auto &a : allAssessments)
    {
        if (a.getId() == assessmentId)
        {
            courseId = a.getCourseId();
            break;
        }
    }

    if (courseId == -1)
        return;

    std::unique_ptr<Course> c(myManager->getCourse(courseId));

    QVector<Student *> students;
    if (c)
    {
        students = myManager->getStudentsBySemester(c->getSemester());
    }

    for (int i = 0; i < students.size(); ++i)
    {
        ui->tableGrading->insertRow(i);
        ui->tableGrading->setItem(i, 0, new QTableWidgetItem(QString::number(students[i]->getId())));
        ui->tableGrading->setItem(i, 1, new QTableWidgetItem(students[i]->getName()));

        double currentGrade = myManager->getGrade(students[i]->getId(), assessmentId);
        QString gradeStr = (currentGrade >= 0) ? QString::number(currentGrade) : "";
        ui->tableGrading->setItem(i, 2, new QTableWidgetItem(gradeStr));
    }
    qDeleteAll(students);
    Utils::adjustColumnWidths(ui->tableGrading);
}

void UIAcademics::onSaveGradesClicked()
{
    int assessmentId = ui->comboTeacherAssessment->currentData().toInt();
    QVector<Assessment> assessments = myManager->getAssessments();
    int maxMarks = -1;
    int courseId = -1;
    for (const auto &a : assessments)
    {
        if (a.getId() == assessmentId)
        {
            maxMarks = a.getMaxMarks();
            courseId = a.getCourseId();
            break;
        }
    }

    if (courseId == -1)
    {
        QMessageBox::warning(nullptr, "Error", "Invalid assessment selected.");
        return;
    }

    Course *course = myManager->getCourse(courseId);
    if (!course || course->getTeacherId() != userId)
    {
        QMessageBox::warning(nullptr, "Permission Denied", "You don't have permission to grade this assessment.");
        delete course;
        return;
    }
    delete course;

    int rows = ui->tableGrading->rowCount();
    for (int i = 0; i < rows; ++i)
    {
        int sid = ui->tableGrading->item(i, 0)->text().toInt();
        QString text = ui->tableGrading->item(i, 2)->text();
        if (text.isEmpty())
            continue;

        double marks = text.toDouble();
        if (marks < 0 || (maxMarks >= 0 && marks > maxMarks))
        {
            QMessageBox::warning(nullptr, "Invalid Grade", QString("Student %1: Grade must be between 0 and %2").arg(sid).arg(maxMarks >= 0 ? QString::number(maxMarks) : "?"));
            return;
        }

        myManager->addGrade(sid, assessmentId, marks);
    }
    QMessageBox::information(nullptr, "Success", "Grades Saved");
}

void UIAcademics::refreshTeacherAttendance()
{
    ui->tableAttendance->clear();
    int courseId = 0;
    if (ui->comboAttendanceCourse->count() > 0)
    {
        courseId = ui->comboAttendanceCourse->currentData().toInt();
    }
    else
    {
        return;
    }
    std::unique_ptr<Course> c(myManager->getCourse(courseId));
    if (!c)
        return;

    QVector<Student *> students = myManager->getStudentsBySemester(c->getSemester());
    QVector<QString> dates = myManager->getCourseDates(courseId);

    QStringList headers;
    headers << "ID" << "Name" << "%" << "Total";
    for (const QString &d : dates)
        headers << d;

    ui->tableAttendance->setColumnCount(headers.size());
    ui->tableAttendance->setHorizontalHeaderLabels(headers);
    ui->tableAttendance->setRowCount(students.size());

    for (int i = 0; i < students.size(); ++i)
    {
        int sid = students[i]->getId();
        ui->tableAttendance->setItem(i, 0, new QTableWidgetItem(QString::number(sid)));
        ui->tableAttendance->setItem(i, 1, new QTableWidgetItem(students[i]->getName()));

        int presentCount = 0;
        for (int j = 0; j < dates.size(); ++j)
        {
            bool present = myManager->isPresent(courseId, sid, dates[j]);
            if (present)
                presentCount++;

            QTableWidgetItem *checkItem = new QTableWidgetItem();
            checkItem->setCheckState(present ? Qt::Checked : Qt::Unchecked);
            ui->tableAttendance->setItem(i, 4 + j, checkItem);
        }

        double pct = dates.isEmpty() ? 0.0 : (double)presentCount / dates.size() * 100.0;
        QTableWidgetItem *pctItem = new QTableWidgetItem(QString::number(pct, 'f', 1) + "%");
        if (pct < 60.0)
            pctItem->setForeground(QColor(220, 20, 60));
        else if (pct < 75.0)
            pctItem->setForeground(QColor(255, 165, 0));
        ui->tableAttendance->setItem(i, 2, pctItem);
        ui->tableAttendance->setItem(i, 3, new QTableWidgetItem(QString::number(presentCount) + "/" + QString::number(dates.size())));
    }

    qDeleteAll(students);
    Utils::adjustColumnWidths(ui->tableAttendance);
}

void UIAcademics::onAttendanceCourseChanged(int index)
{
    refreshTeacherAttendance();
}

void UIAcademics::onAddClassDateClicked()
{
    int courseId = ui->comboAttendanceCourse->currentData().toInt();
    if (courseId <= 0)
        return;

    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    bool ok;
    QString inputDate = QInputDialog::getText(nullptr, "Add Class Date", "Date (yyyy-MM-dd):", QLineEdit::Normal, date, &ok);
    if (!ok || inputDate.isEmpty())
        return;

    QDate parsed = QDate::fromString(inputDate, "yyyy-MM-dd");
    if (!parsed.isValid())
    {
        QMessageBox::warning(nullptr, "Invalid Date", "Please enter a valid date in yyyy-MM-dd format.");
        return;
    }

    QVector<QString> existingDates = myManager->getCourseDates(courseId);
    if (existingDates.contains(inputDate))
    {
        QMessageBox::warning(nullptr, "Duplicate Date", "This date already exists for the selected course.");
        return;
    }

    std::unique_ptr<Course> c(myManager->getCourse(courseId));
    if (!c)
        return;

    QVector<Student *> students = myManager->getStudentsBySemester(c->getSemester());
    for (auto s : students)
    {
        myManager->markAttendance(courseId, s->getId(), inputDate, false);
    }
    qDeleteAll(students);

    refreshTeacherAttendance();
    QMessageBox::information(nullptr, "Success", "Class date added: " + inputDate);
}

void UIAcademics::onSaveAttendanceClicked()
{
    int courseId = ui->comboAttendanceCourse->currentData().toInt();
    if (courseId <= 0)
        return;

    int rows = ui->tableAttendance->rowCount();
    int cols = ui->tableAttendance->columnCount();

    for (int j = 4; j < cols; ++j)
    {
        QString date = ui->tableAttendance->horizontalHeaderItem(j)->text();
        for (int i = 0; i < rows; ++i)
        {
            int sid = ui->tableAttendance->item(i, 0)->text().toInt();
            bool present = (ui->tableAttendance->item(i, j)->checkState() == Qt::Checked);
            myManager->markAttendance(courseId, sid, date, present);
        }
    }
    QMessageBox::information(nullptr, "Success", "Attendance Saved");
}

void UIAcademics::onCheckAttendanceWarningsClicked()
{
    if (ui->comboAttendanceCourse->count() == 0)
    {
        Notifications::warning(nullptr, "No courses available.");
        return;
    }

    int courseId = ui->comboAttendanceCourse->currentData().toInt();
    if (courseId <= 0)
    {
        Notifications::warning(nullptr, "Please select a course first.");
        return;
    }

    int count = myManager->generateAttendanceWarnings(courseId, userId);

    if (count == 0)
        Notifications::info(nullptr, "All students have sufficient attendance (>= 75%) in this course.");
    else
        Notifications::success(nullptr, QString("Generated %1 attendance warning notice(s) for students below 75% attendance.").arg(count));
}
