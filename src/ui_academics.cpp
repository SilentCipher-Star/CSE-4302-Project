#include "../include/ui_academics.hpp"
#include "../include/utils.hpp"
#include "../include/course.hpp"
#include "../include/student.hpp"
#include "../include/notifications.hpp"
#include "../include/exceptions.hpp"
#include "../include/gpa_strategy.hpp"
#include "../include/report.hpp"
#include "../include/student_iterator.hpp"
#include "../include/attendance_simulator.hpp"
#include "../include/ui_teacher_attendance.hpp"
#include "../include/ui_personal_inbox.hpp"
#include "../include/commands.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDate>
#include <QHeaderView>
#include <memory>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QLineEdit>
#include <QTextEdit>

namespace
{
    // Helper to fetch assessment details to avoid duplicating this logic.
    struct AssessmentDetails
    {
        int courseId = -1;
        int maxMarks = -1;
        bool isValid() const { return courseId != -1; }
    };

    AssessmentDetails getDetails(AcadenceManager *manager, int assessmentId)
    {
        AssessmentDetails details;
        QVector<Assessment> assessments = manager->getAssessments();
        for (const auto &a : assessments)
        {
            if (a.getId() == assessmentId)
            {
                details.courseId = a.getCourseId();
                details.maxMarks = a.getMaxMarks();
                return details;
            }
        }
        return details;
    }
}

UIAcademics::UIAcademics(Ui::MainWindow *ui, AcadenceManager *manager, QString role, int uid, QObject *parent)
    : QObject(parent), ui(ui), myManager(manager), userRole(role), userId(uid), btnCheckWarnings(nullptr)
{
    if (role == "Teacher")
    {
        btnCheckWarnings = new QPushButton("Check Attendance Warnings");
        ui->hbox_attendance_controls->addWidget(btnCheckWarnings);
        connect(btnCheckWarnings, &QPushButton::clicked, this, &UIAcademics::onCheckAttendanceWarningsClicked);

        btnDailyAttendance = new QPushButton("Daily Attendance Tracker");
        btnDailyAttendance->setStyleSheet("padding: 6px 14px; font-weight: bold;");
        ui->hbox_attendance_controls->addWidget(btnDailyAttendance);
        connect(btnDailyAttendance, &QPushButton::clicked, this, &UIAcademics::onDailyAttendanceClicked);

        btnSendGradeReports = new QPushButton("Send Grade Reports");
        btnSendGradeReports->setStyleSheet("padding: 6px 14px; font-weight: bold;");
        ui->hbox_attendance_controls->addWidget(btnSendGradeReports);
        connect(btnSendGradeReports, &QPushButton::clicked, this, &UIAcademics::onSendGradeReportsClicked);

        btnSendMessage = new QPushButton("Send Personal Message");
        btnSendMessage->setStyleSheet("padding: 6px 14px;");
        ui->hbox_attendance_controls->addWidget(btnSendMessage);
        connect(btnSendMessage, &QPushButton::clicked, this, &UIAcademics::onSendPersonalMessageClicked);
    }
    else if (role == "Student")
    {
        ui->listAssessments->setMinimumHeight(160);
        ui->listAssessments->setMaximumHeight(260);

        QHBoxLayout *btnRow = new QHBoxLayout();

        btnGPACalc = new QPushButton("GPA Calculator");
        btnGPACalc->setMaximumWidth(200);
        btnRow->addWidget(btnGPACalc);
        connect(btnGPACalc, &QPushButton::clicked, this, &UIAcademics::onGPACalculatorClicked);

        btnExport = new QPushButton("Export Report");
        btnExport->setMaximumWidth(200);
        btnRow->addWidget(btnExport);
        connect(btnExport, &QPushButton::clicked, this, &UIAcademics::onExportReportClicked);

        btnAttendSim = new QPushButton("Attendance Simulator");
        btnAttendSim->setMaximumWidth(220);
        btnRow->addWidget(btnAttendSim);
        connect(btnAttendSim, &QPushButton::clicked, this, &UIAcademics::onAttendanceSimulatorClicked);

        btnInbox = new QPushButton("Inbox");
        btnInbox->setMaximumWidth(200);
        btnRow->addWidget(btnInbox);
        connect(btnInbox, &QPushButton::clicked, this, &UIAcademics::onOpenInboxClicked);

        btnRow->addStretch();

        ui->verticalLayout_academics->addLayout(btnRow);
    }
}

void UIAcademics::refreshAcademics()
{
    ui->listAssessments->clear();
    QVector<Assessment> assessments = myManager->getStudentAssessments(userId);
    QDate today = QDate::currentDate();
    for (const auto &a : assessments)
    {
        QDate assessDate = QDate::fromString(a.getDate(), "yyyy-MM-dd");
        QString countdownText;
        QColor itemColor;

        if (assessDate.isValid())
        {
            int daysLeft = today.daysTo(assessDate);
            if (daysLeft < 0)
            {
                countdownText = " [Past]";
                itemColor = QColor(150, 150, 150);
            }
            else if (daysLeft == 0)
            {
                countdownText = " [TODAY]";
                itemColor = QColor(220, 20, 60);
            }
            else if (daysLeft <= 3)
            {
                countdownText = QString(" [%1 day(s) left]").arg(daysLeft);
                itemColor = QColor(220, 20, 60);
            }
            else if (daysLeft <= 7)
            {
                countdownText = QString(" [%1 day(s) left]").arg(daysLeft);
                itemColor = QColor(255, 140, 0);
            }
            else
            {
                countdownText = QString(" [%1 day(s) left]").arg(daysLeft);
                itemColor = QColor(34, 139, 34);
            }
        }

        QString text = a.getDate() + " - " + a.getCourseName() + ": " + a.getTitle() + " (" + a.getType() + ")" + countdownText;
        QListWidgetItem *item = new QListWidgetItem(text);
        if (itemColor.isValid())
            item->setForeground(itemColor);
        ui->listAssessments->addItem(item);
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

    bool hasWarning = false;
    QString warningCourses;
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

        if (pct >= 75.0)
        {
            status = "Good";
            rowColor = QColor(34, 139, 34);
        }
        else if (pct >= 60.0)
        {
            status = "At Risk";
            rowColor = QColor(255, 165, 0);
            hasWarning = true;
            if (!warningCourses.isEmpty())
                warningCourses += ", ";
            warningCourses += att[i].getCourseName();
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
    ui->tableAcademics->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

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
    else if (hasWarning)
    {
        Notifications::warning(nullptr,
            "Attendance below 75% in: " + warningCourses +
            ". You are at risk of being barred from exams.");
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

    const int assessmentId = ui->comboTeacherAssessment->currentData().toInt();
    if (assessmentId <= 0)
        return;

    const auto details = getDetails(myManager, assessmentId);
    if (!details.isValid())
        return;

    std::unique_ptr<Course> c(myManager->getCourse(details.courseId));

    QVector<Student *> students;
    if (c)
    {
        students = myManager->getStudentsBySemester(c->getSemester());
    }

    StudentCollection collection;
    for (auto *s : students)
        collection.add(s);

    StudentIterator iter = collection.createIterator();
    int i = 0;
    while (iter.hasNext())
    {
        Student *stu = iter.next();
        ui->tableGrading->insertRow(i);
        ui->tableGrading->setItem(i, 0, new QTableWidgetItem(QString::number(stu->getId())));
        ui->tableGrading->setItem(i, 1, new QTableWidgetItem(stu->getName()));

        double currentGrade = myManager->getGrade(stu->getId(), assessmentId);
        QString gradeStr = (currentGrade >= 0) ? QString::number(currentGrade) : "";
        ui->tableGrading->setItem(i, 2, new QTableWidgetItem(gradeStr));
        ++i;
    }
    qDeleteAll(students);
    ui->tableGrading->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

void UIAcademics::onSaveGradesClicked()
{
    int assessmentId = ui->comboTeacherAssessment->currentData().toInt();
    const auto details = getDetails(myManager, assessmentId);

    if (!details.isValid())
    {
        QMessageBox::warning(nullptr, "Error", "Invalid assessment selected.");
        return;
    }

    const int maxMarks = details.maxMarks;
    const int courseId = details.courseId;

    std::unique_ptr<Course> course(myManager->getCourse(courseId));
    if (!course || course->getTeacherId() != userId)
    {
        QMessageBox::warning(nullptr, "Permission Denied", "You don't have permission to grade this assessment.");
        return;
    }

    int rows = ui->tableGrading->rowCount();
    QVector<CommandPtr> gradeCommands;

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

        gradeCommands.append(std::make_shared<AddGradeCommand>(sid, assessmentId, marks));
    }

    if (!gradeCommands.isEmpty())
    {
        auto batch = std::make_shared<BatchCommand>(gradeCommands, "Save grades", DataType::Academics);
        myManager->executeCommand(batch);
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
    QVector<CommandPtr> attCommands;
    for (auto s : students)
    {
        attCommands.append(std::make_shared<MarkAttendanceCommand>(courseId, s->getId(), inputDate, false));
    }
    qDeleteAll(students);

    if (!attCommands.isEmpty())
    {
        auto batch = std::make_shared<BatchCommand>(attCommands, "Add class date " + inputDate, DataType::Academics);
        myManager->executeCommand(batch);
    }

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

    QVector<CommandPtr> attCommands;
    for (int j = 4; j < cols; ++j)
    {
        QString date = ui->tableAttendance->horizontalHeaderItem(j)->text();
        for (int i = 0; i < rows; ++i)
        {
            int sid = ui->tableAttendance->item(i, 0)->text().toInt();
            bool present = (ui->tableAttendance->item(i, j)->checkState() == Qt::Checked);
            attCommands.append(std::make_shared<MarkAttendanceCommand>(courseId, sid, date, present));
        }
    }

    if (!attCommands.isEmpty())
    {
        auto batch = std::make_shared<BatchCommand>(attCommands, "Save attendance", DataType::Academics);
        myManager->executeCommand(batch);
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

void UIAcademics::onGPACalculatorClicked()
{
    QVector<AttendanceRecord> att = myManager->getStudentAttendance(userId);

    double totalMarks = 0.0;
    double totalMax = 0.0;
    int courseCount = 0;
    for (const auto &rec : att)
    {
        if (rec.getTotalMaxMarks() > 0)
        {
            totalMarks += rec.getTotalMarksObtained();
            totalMax   += rec.getTotalMaxMarks();
            ++courseCount;
        }
    }

    double currentPct = (totalMax > 0) ? (totalMarks / totalMax * 100.0) : 0.0;

    Acadence::PercentageGPAStrategy pctStrategy;
    Acadence::LetterGradeGPAStrategy letterStrategy;

    double currentGPA = pctStrategy.calculate(currentPct);

    QDialog dlg;
    dlg.setWindowTitle("GPA Calculator");
    dlg.setMinimumWidth(400);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    QHBoxLayout *schemeRow = new QHBoxLayout();
    schemeRow->addWidget(new QLabel("GPA Scheme:"));
    QComboBox *comboScheme = new QComboBox();
    comboScheme->addItem(pctStrategy.schemeName());
    comboScheme->addItem(letterStrategy.schemeName());
    schemeRow->addWidget(comboScheme);
    layout->addLayout(schemeRow);

    QLabel *lblCurrent = new QLabel();
    lblCurrent->setStyleSheet("font-size: 13px; font-weight: bold; padding: 8px;");
    layout->addWidget(lblCurrent);

    auto updateCurrentLabel = [&](double gpa) {
        lblCurrent->setText(
            QString("Current Average: %1%\nEstimated GPA: %2 / 4.0\nCourses Completed: %3")
                .arg(QString::number(currentPct, 'f', 1))
                .arg(QString::number(gpa, 'f', 2))
                .arg(courseCount));
    };
    updateCurrentLabel(currentGPA);

    QLabel *sep = new QLabel("── What-If Simulator ──");
    sep->setAlignment(Qt::AlignCenter);
    layout->addWidget(sep);

    QHBoxLayout *targetRow = new QHBoxLayout();
    targetRow->addWidget(new QLabel("Target GPA:"));
    QDoubleSpinBox *spinTarget = new QDoubleSpinBox();
    spinTarget->setRange(0.0, 4.0);
    spinTarget->setSingleStep(0.1);
    spinTarget->setDecimals(2);
    spinTarget->setValue(3.5);
    targetRow->addWidget(spinTarget);
    layout->addLayout(targetRow);

    QHBoxLayout *remRow = new QHBoxLayout();
    remRow->addWidget(new QLabel("Remaining Courses:"));
    QSpinBox *spinRem = new QSpinBox();
    spinRem->setRange(1, 40);
    spinRem->setValue(4);
    remRow->addWidget(spinRem);
    layout->addLayout(remRow);

    QLabel *lblResult = new QLabel("");
    lblResult->setStyleSheet("font-size: 13px; padding: 8px; color: #1a73e8;");
    lblResult->setWordWrap(true);
    layout->addWidget(lblResult);

    QPushButton *btnCalc = new QPushButton("Calculate");
    layout->addWidget(btnCalc);

    QObject::connect(comboScheme, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int idx) {
        double gpa = (idx == 0) ? pctStrategy.calculate(currentPct) : letterStrategy.calculate(currentPct);
        updateCurrentLabel(gpa);
        lblResult->setText("");
    });

    QObject::connect(btnCalc, &QPushButton::clicked, [&]() {
        int schemeIdx = comboScheme->currentIndex();
        Acadence::IGPAStrategy *strategy = (schemeIdx == 0)
            ? static_cast<Acadence::IGPAStrategy*>(&pctStrategy)
            : static_cast<Acadence::IGPAStrategy*>(&letterStrategy);

        double usedGPA = strategy->calculate(currentPct);
        double targetGPA = spinTarget->value();
        int remaining = spinRem->value();
        int total = courseCount + remaining;

        double requiredTotal    = targetGPA * total;
        double requiredFromRem  = requiredTotal - usedGPA * courseCount;
        double requiredPerCourse = requiredFromRem / remaining;

        if (requiredPerCourse > 4.0)
        {
            lblResult->setText("Target GPA is not achievable with the given number of remaining courses.");
            lblResult->setStyleSheet("font-size: 13px; padding: 8px; color: #d32f2f;");
        }
        else if (requiredPerCourse < 0.0)
        {
            lblResult->setText("You have already exceeded your target GPA!");
            lblResult->setStyleSheet("font-size: 13px; padding: 8px; color: #2e7d32;");
        }
        else
        {
            lblResult->setText(
                QString("Using scheme: %1\nYou need an average GPA of %2 per course\nin your remaining %3 course(s).")
                    .arg(strategy->schemeName())
                    .arg(QString::number(requiredPerCourse, 'f', 2))
                    .arg(remaining));
            lblResult->setStyleSheet("font-size: 13px; padding: 8px; color: #1a73e8;");
        }
    });

    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Close);
    layout->addWidget(bbox);
    QObject::connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::accept);

    dlg.exec();
}

void UIAcademics::onExportReportClicked()
{
    bool ok;
    QStringList formats = {"CSV (.csv)", "Text (.txt)"};
    QString chosen = QInputDialog::getItem(nullptr, "Export Format", "Select format:", formats, 0, false, &ok);
    if (!ok)
        return;

    bool isCsv = chosen.startsWith("CSV");
    QString filter = isCsv ? "CSV Files (*.csv)" : "Text Files (*.txt)";
    QString defaultName = isCsv ? "academic_report.csv" : "academic_report.txt";

    QString path = QFileDialog::getSaveFileName(nullptr, "Export Report", defaultName, filter);
    if (path.isEmpty())
        return;

    Student *stu = myManager->getStudent(userId);
    QString studentName = stu ? stu->getName() : QString::number(userId);
    delete stu;

    QVector<AttendanceRecord> att = myManager->getStudentAttendance(userId);
    QVector<Assessment> assessments = myManager->getStudentAssessments(userId);

    std::unique_ptr<Acadence::IReport> report;
    if (isCsv)
        report = std::make_unique<Acadence::CSVReport>();
    else
        report = std::make_unique<Acadence::TextReport>();

    report->setData(studentName, att, assessments);
    report->generate(path);

    QMessageBox::information(nullptr, "Export Successful",
        QString("%1 report exported to:\n%2").arg(report->formatName()).arg(path));
}

void UIAcademics::onAttendanceSimulatorClicked()
{
    QVector<AttendanceRecord> att = myManager->getStudentAttendance(userId);

    if (att.isEmpty())
    {
        QMessageBox::information(nullptr, "No Data", "No attendance records found.");
        return;
    }

    QVector<QPair<QString, QPair<int,int>>> courseData;
    for (const auto &rec : att)
    {
        courseData.append({rec.getCourseName(), {rec.getAttendedClasses(), rec.getTotalClasses()}});
    }

    AttendanceSimulatorDialog dlg(courseData, nullptr);
    dlg.exec();
}

void UIAcademics::onDailyAttendanceClicked()
{
    TeacherAttendanceDialog dlg(myManager, userId, nullptr);
    dlg.exec();
    refreshTeacherAttendance();
}

void UIAcademics::onSendGradeReportsClicked()
{
    if (ui->comboTeacherAssessment->count() == 0)
    {
        QMessageBox::warning(nullptr, "No Assessments", "No assessments found. Create an assessment first.");
        return;
    }

    int assessmentId = ui->comboTeacherAssessment->currentData().toInt();
    QString assessName = ui->comboTeacherAssessment->currentText();

    // Ask for optional note
    bool ok;
    QString note = QInputDialog::getMultiLineText(nullptr, "Send Grade Reports",
        QString("Sending personal grade reports for:\n%1\n\nAdd an optional note to all students (leave empty for none):").arg(assessName),
        "", &ok);
    if (!ok) return;

    int count = myManager->sendBulkGradeReports(userId, assessmentId, note);

    if (count > 0)
        QMessageBox::information(nullptr, "Sent",
            QString("Successfully sent %1 personal grade report(s).\n\nEach student will see their own marks privately in their Inbox.").arg(count));
    else
        QMessageBox::warning(nullptr, "Failed", "No grade reports could be sent. Make sure students are enrolled.");
}

void UIAcademics::onSendPersonalMessageClicked()
{
    // Get list of courses to find students
    QVector<Course *> courses = myManager->getTeacherCourses(userId);
    if (courses.isEmpty())
    {
        QMessageBox::warning(nullptr, "No Courses", "You have no courses assigned.");
        return;
    }

    QDialog dlg;
    dlg.setWindowTitle("Send Personal Message");
    dlg.setMinimumWidth(450);

    QVBoxLayout *layout = new QVBoxLayout(&dlg);

    // Course selection
    layout->addWidget(new QLabel("Select Course:"));
    QComboBox *comboCourse = new QComboBox();
    for (auto *c : courses)
        comboCourse->addItem(c->getCode() + " - " + c->getName(), c->getId());
    layout->addWidget(comboCourse);

    // Student selection
    layout->addWidget(new QLabel("Select Student:"));
    QComboBox *comboStudent = new QComboBox();
    layout->addWidget(comboStudent);

    // Populate students when course changes
    auto populateStudents = [&](int index) {
        comboStudent->clear();
        if (index < 0) return;
        int cid = comboCourse->currentData().toInt();
        std::unique_ptr<Course> c(myManager->getCourse(cid));
        if (!c) return;

        QVector<Student *> students = myManager->getStudentsByEnrollment(cid);
        if (students.isEmpty())
            students = myManager->getStudentsBySemester(c->getSemester());

        comboStudent->addItem("-- All Students --", -1);
        for (auto *s : students)
            comboStudent->addItem(QString("%1 - %2").arg(s->getId()).arg(s->getName()), s->getId());
        qDeleteAll(students);
    };
    connect(comboCourse, QOverload<int>::of(&QComboBox::currentIndexChanged), populateStudents);
    populateStudents(0);

    // Subject
    layout->addWidget(new QLabel("Subject:"));
    QLineEdit *editSubject = new QLineEdit();
    editSubject->setPlaceholderText("e.g., Regarding your performance...");
    layout->addWidget(editSubject);

    // Message body
    layout->addWidget(new QLabel("Message:"));
    QTextEdit *editBody = new QTextEdit();
    editBody->setMinimumHeight(120);
    editBody->setPlaceholderText("Type your message here...");
    layout->addWidget(editBody);

    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    bbox->button(QDialogButtonBox::Ok)->setText("Send");
    layout->addWidget(bbox);
    connect(bbox, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(bbox, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    qDeleteAll(courses);

    if (dlg.exec() != QDialog::Accepted) return;

    QString subject = editSubject->text().trimmed();
    QString body = editBody->toPlainText().trimmed();
    if (subject.isEmpty() || body.isEmpty())
    {
        QMessageBox::warning(nullptr, "Incomplete", "Please provide both subject and message.");
        return;
    }

    int targetStudentId = comboStudent->currentData().toInt();

    if (targetStudentId == -1)
    {
        // Send to all students in course
        int cid = comboCourse->currentData().toInt();
        std::unique_ptr<Course> c(myManager->getCourse(cid));
        if (!c) return;

        QVector<Student *> students = myManager->getStudentsByEnrollment(cid);
        if (students.isEmpty())
            students = myManager->getStudentsBySemester(c->getSemester());

        for (auto *s : students)
            myManager->sendMessage(userId, "Teacher", s->getId(), subject, body);

        int count = students.size();
        qDeleteAll(students);
        QMessageBox::information(nullptr, "Sent", QString("Message sent to %1 student(s).").arg(count));
    }
    else
    {
        myManager->sendMessage(userId, "Teacher", targetStudentId, subject, body);
        QMessageBox::information(nullptr, "Sent", "Message sent successfully!");
    }
}

void UIAcademics::onOpenInboxClicked()
{
    PersonalInboxDialog dlg(myManager, userId, userRole, nullptr);
    dlg.exec();

    // Update inbox button text with unread count
    int unread = myManager->getUnreadMessageCount(userId);
    if (btnInbox)
    {
        if (unread > 0)
            btnInbox->setText(QString("Inbox (%1)").arg(unread));
        else
            btnInbox->setText("Inbox");
    }
}
