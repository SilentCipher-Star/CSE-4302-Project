#include "../include/ui_teacher_attendance.hpp"
#include "../include/theme.hpp"
#include "../include/course.hpp"
#include "../include/student.hpp"
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QFont>
#include <QFrame>
#include <QInputDialog>
#include <QScrollBar>
#include <algorithm>
#include <memory>
#include "../include/commands.hpp"

TeacherAttendanceDialog::TeacherAttendanceDialog(AcadenceManager *manager, int teacherId, QWidget *parent)
    : QDialog(parent), myManager(manager), teacherId(teacherId)
{
    setWindowTitle("Daily Attendance Tracker");
    setMinimumSize(950, 600);
    resize(1150, 680);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // Add title for the dialog
    QLabel *title = new QLabel("Daily Attendance Sheet");
    title->setStyleSheet(QString("font-size: %1px; font-weight: bold; padding: 4px 0;").arg(AppFonts::Large));
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Layout for course selection and class statistics
    QHBoxLayout *topBar = new QHBoxLayout();

    topBar->addWidget(new QLabel("Course:"));
    comboCourse = new QComboBox();
    comboCourse->setMinimumWidth(260);
    topBar->addWidget(comboCourse);

    topBar->addSpacing(20);
    lblClassesHeld = new QLabel("Number of Classes Held: 0");
    lblClassesHeld->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #1a73e8;").arg(AppFonts::Normal));
    topBar->addWidget(lblClassesHeld);

    topBar->addStretch();

    lblStats = new QLabel();
    lblStats->setStyleSheet(QString("font-size: %1px; font-weight: bold;").arg(AppFonts::Small));
    topBar->addWidget(lblStats);

    mainLayout->addLayout(topBar);

    // Controls to navigate between weeks
    QHBoxLayout *navBar = new QHBoxLayout();

    btnPrevWeek = new QPushButton("<<  Prev Week");
    btnPrevWeek->setFixedWidth(130);
    btnPrevWeek->setStyleSheet("padding: 5px 10px;");
    navBar->addWidget(btnPrevWeek);

    lblWeekRange = new QLabel();
    lblWeekRange->setAlignment(Qt::AlignCenter);
    lblWeekRange->setStyleSheet(QString("font-size: %1px; font-weight: bold;").arg(AppFonts::Normal));
    navBar->addWidget(lblWeekRange, 1);

    btnNextWeek = new QPushButton("Next Week  >>");
    btnNextWeek->setFixedWidth(130);
    btnNextWeek->setStyleSheet("padding: 5px 10px;");
    navBar->addWidget(btnNextWeek);

    mainLayout->addLayout(navBar);

    // Visual separator
    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #444;");
    mainLayout->addWidget(sep);

    // Buttons to manipulate attendance data
    QHBoxLayout *actionBar = new QHBoxLayout();

    btnAddToday = new QPushButton("+ Add Today's Class");
    btnAddToday->setStyleSheet("padding: 6px 14px; background-color: #1a73e8; color: white; border-radius: 4px; font-weight: bold;");
    actionBar->addWidget(btnAddToday);

    btnMarkAllPresent = new QPushButton("Mark All Present");
    btnMarkAllPresent->setStyleSheet("padding: 6px 14px; background-color: #22a85a; color: white; border-radius: 4px;");
    actionBar->addWidget(btnMarkAllPresent);

    btnMarkAllAbsent = new QPushButton("Mark All Absent");
    btnMarkAllAbsent->setStyleSheet("padding: 6px 14px; background-color: #dc143c; color: white; border-radius: 4px;");
    actionBar->addWidget(btnMarkAllAbsent);

    actionBar->addSpacing(10);

    btnMarkColPresent = new QPushButton("Column -> Present");
    btnMarkColPresent->setToolTip("Click a date column header first, then use this to mark all students Present for that date");
    btnMarkColPresent->setStyleSheet("padding: 6px 12px;");
    actionBar->addWidget(btnMarkColPresent);

    btnMarkColAbsent = new QPushButton("Column -> Absent");
    btnMarkColAbsent->setToolTip("Click a date column header first, then use this to mark all students Absent for that date");
    btnMarkColAbsent->setStyleSheet("padding: 6px 12px;");
    actionBar->addWidget(btnMarkColAbsent);

    actionBar->addStretch();
    mainLayout->addLayout(actionBar);

    // Legend indicating cell color meanings
    QHBoxLayout *legendRow = new QHBoxLayout();
    auto addLegend = [&](const QString &bgColor, const QString &text)
    {
        QLabel *box = new QLabel();
        box->setFixedSize(18, 18);
        box->setStyleSheet(QString("background-color: %1; border: 1px solid #555; border-radius: 2px;").arg(bgColor));
        legendRow->addWidget(box);
        QLabel *lbl = new QLabel(text);
        lbl->setStyleSheet(QString("font-size: %1px;").arg(AppFonts::Small));
        legendRow->addWidget(lbl);
        legendRow->addSpacing(14);
    };
    addLegend("#22a85a", "Present (P)");
    addLegend("#dc143c", "Absent (A)");
    addLegend("#555", "No Class");
    legendRow->addStretch();
    QLabel *hint = new QLabel("Click cells to toggle | Click column headers for column operations");
    hint->setStyleSheet(QString("font-size: %1px; color: palette(text);").arg(AppFonts::Small));
    legendRow->addWidget(hint);
    mainLayout->addLayout(legendRow);

    // Setup main attendance grid
    table = new QTableWidget();
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    table->setStyleSheet(QString(
                             "QTableWidget { gridline-color: palette(mid); font-size: %1px; }"
                             "QTableWidget::item { padding: 4px; }"
                             "QHeaderView::section { font-weight: bold; padding: 4px; }")
                             .arg(AppFonts::Small));
    mainLayout->addWidget(table, 1);

    // Bottom controls for exporting and saving
    QHBoxLayout *btnBar = new QHBoxLayout();

    btnExport = new QPushButton("Export CSV");
    btnExport->setStyleSheet("padding: 6px 14px;");
    btnBar->addWidget(btnExport);

    btnBar->addStretch();

    btnSave = new QPushButton("Save Attendance");
    btnSave->setStyleSheet("padding: 6px 20px; font-weight: bold; background-color: #22a85a; color: white; border-radius: 4px;");
    btnBar->addWidget(btnSave);

    QPushButton *btnClose = new QPushButton("Close");
    btnClose->setFixedWidth(100);
    btnClose->setStyleSheet("padding: 6px 14px;");
    btnBar->addWidget(btnClose);

    mainLayout->addLayout(btnBar);

    // Fill dropdown with assigned courses
    auto courses = myManager->getTeacherCourses(teacherId);
    for (const auto &c : courses)
    {
        comboCourse->addItem(c->getCode() + " - " + c->getName(), c->getId());
    }

    // Connect UI events
    connect(comboCourse, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TeacherAttendanceDialog::onCourseChanged);
    connect(table, &QTableWidget::cellClicked, this, &TeacherAttendanceDialog::onCellClicked);
    connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &TeacherAttendanceDialog::onHeaderClicked);
    connect(table, &QTableWidget::customContextMenuRequested, this, &TeacherAttendanceDialog::onTableContextMenu);
    connect(btnPrevWeek, &QPushButton::clicked, this, &TeacherAttendanceDialog::onWeekPrev);
    connect(btnNextWeek, &QPushButton::clicked, this, &TeacherAttendanceDialog::onWeekNext);
    connect(btnAddToday, &QPushButton::clicked, this, &TeacherAttendanceDialog::onAddTodayClicked);
    connect(btnMarkAllPresent, &QPushButton::clicked, this, &TeacherAttendanceDialog::onMarkAllPresentClicked);
    connect(btnMarkAllAbsent, &QPushButton::clicked, this, &TeacherAttendanceDialog::onMarkAllAbsentClicked);
    connect(btnMarkColPresent, &QPushButton::clicked, this, &TeacherAttendanceDialog::onMarkColumnPresentClicked);
    connect(btnMarkColAbsent, &QPushButton::clicked, this, &TeacherAttendanceDialog::onMarkColumnAbsentClicked);
    connect(btnSave, &QPushButton::clicked, this, &TeacherAttendanceDialog::onSaveClicked);
    connect(btnExport, &QPushButton::clicked, this, &TeacherAttendanceDialog::onExportClicked);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);

    // Trigger initial population
    if (comboCourse->count() > 0)
        onCourseChanged(0);
}

TeacherAttendanceDialog::~TeacherAttendanceDialog()
{
}

int TeacherAttendanceDialog::visibleDateCount() const
{
    int start = (int)allDates.size() - datesPerPage - weekOffset * datesPerPage;
    if (start < 0)
        start = 0;
    int end = start + datesPerPage;
    if (end > allDates.size())
        end = allDates.size();
    return end - start;
}

QVector<QString> TeacherAttendanceDialog::visibleDates() const
{
    if (allDates.isEmpty())
        return {};

    int total = allDates.size();
    int start = total - datesPerPage - weekOffset * datesPerPage;
    if (start < 0)
        start = 0;
    int end = start + datesPerPage;
    if (end > total)
        end = total;

    QVector<QString> result;
    for (int i = start; i < end; ++i)
        result.append(allDates[i]);
    return result;
}

void TeacherAttendanceDialog::buildTable()
{
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(0);
    selectedDateCol = -1;

    if (currentCourseId <= 0)
        return;

    QVector<QString> dates = visibleDates();
    int dateCols = dates.size();
    int totalCols = 2 + dateCols + 2; // ID, Name, [dates...], Absent, Absent%

    QStringList headers;
    headers << "ID" << "Student Name";
    for (const QString &d : dates)
    {
        QDate dt = QDate::fromString(d, "yyyy-MM-dd");
        headers << dt.toString("dd MMM\nddd"); // e.g. "10 Mar\nTue"
    }
    headers << "Absent" << "Absent %";

    table->setColumnCount(totalCols);
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount(students.size());

    // Ensure headers fit multiple lines
    table->horizontalHeader()->setMinimumHeight(40);

    // Apply appropriate column widths
    table->setColumnWidth(0, 50);  // ID
    table->setColumnWidth(1, 160); // Name
    for (int j = 0; j < dateCols; ++j)
        table->setColumnWidth(2 + j, 72);
    table->setColumnWidth(2 + dateCols, 65);     // Absent
    table->setColumnWidth(2 + dateCols + 1, 85); // Absent %

    for (int i = 0; i < students.size(); ++i)
    {
        int sid = students[i]->getId();
        QString name = students[i]->getName().trimmed();

        // Populate student ID
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(sid));
        idItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 0, idItem);

        // Populate student name
        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        table->setItem(i, 1, nameItem);

        // Add clickable presence indicators
        for (int j = 0; j < dateCols; ++j)
        {
            bool present = currentPresenceSet.contains(QString::number(sid) + "_" + dates[j]);

            QTableWidgetItem *cell = new QTableWidgetItem(present ? "P" : "A");
            cell->setTextAlignment(Qt::AlignCenter);
            cell->setData(Qt::UserRole, dates[j]);    // store date
            cell->setData(Qt::UserRole + 1, sid);     // store student id
            cell->setData(Qt::UserRole + 2, present); // store status

            QFont f = cell->font();
            f.setBold(true);
            cell->setFont(f);

            table->setItem(i, 2 + j, cell);
            colorCell(i, 2 + j, present);
        }

        // Compute absence statistics across the whole dataset
        recalcRowStats(i);
    }

    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    updateStatsLabel();

    // Render display range label for active dates
    if (!dates.isEmpty())
    {
        QDate first = QDate::fromString(dates.first(), "yyyy-MM-dd");
        QDate last = QDate::fromString(dates.last(), "yyyy-MM-dd");
        lblWeekRange->setText(
            QString("Showing: %1  to  %2   (%3 of %4 dates)")
                .arg(first.toString("dd MMM yyyy"))
                .arg(last.toString("dd MMM yyyy"))
                .arg(dateCols)
                .arg(allDates.size()));
    }
    else
    {
        lblWeekRange->setText("No class dates yet - click 'Add Today's Class' to start");
    }

    // Toggle pagination limits based on boundaries
    int maxOffset = allDates.isEmpty() ? 0 : (allDates.size() - 1) / datesPerPage;
    btnPrevWeek->setEnabled(weekOffset < maxOffset);
    btnNextWeek->setEnabled(weekOffset > 0);

    lblClassesHeld->setText(QString("Number of Classes Held: %1").arg(allDates.size()));
}

void TeacherAttendanceDialog::colorCell(int row, int col, bool present)
{
    QTableWidgetItem *cell = table->item(row, col);
    if (!cell)
        return;

    if (present)
    {
        cell->setText("P");
        cell->setBackground(QColor(34, 168, 90, 80));
    }
    else
    {
        cell->setText("A");
        cell->setBackground(QColor(220, 20, 60, 80));
    }

    cell->setData(Qt::UserRole + 2, present);
}

void TeacherAttendanceDialog::recalcRowStats(int row)
{
    if (row < 0 || row >= students.size())
        return;

    int sid = students[row]->getId();
    int dateCols = visibleDateCount();
    QVector<QString> visDates = visibleDates();

    int totalAbsent = 0;
    for (const QString &d : allDates)
    {
        int visIdx = visDates.indexOf(d);
        if (visIdx >= 0)
        {
            // Query visible cell presence state
            QTableWidgetItem *c = table->item(row, 2 + visIdx);
            if (c && !c->data(Qt::UserRole + 2).toBool())
                totalAbsent++;
        }
        else
        {
            // Retrieve from backing cache for dates off-screen
            if (!currentPresenceSet.contains(QString::number(sid) + "_" + d))
                totalAbsent++;
        }
    }

    // Record total absence count
    QTableWidgetItem *absentItem = table->item(row, 2 + dateCols);
    if (!absentItem)
    {
        absentItem = new QTableWidgetItem();
        absentItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 2 + dateCols, absentItem);
    }
    absentItem->setText(QString::number(totalAbsent));
    QFont abf = absentItem->font();
    abf.setBold(true);
    absentItem->setFont(abf);
    absentItem->setForeground(totalAbsent > 0 ? QColor(220, 20, 60) : QColor(34, 168, 90));

    // Derive absence percentage ratio
    double absentPct = allDates.isEmpty() ? 0.0 : (double)totalAbsent / allDates.size() * 100.0;
    QTableWidgetItem *pctItem = table->item(row, 2 + dateCols + 1);
    if (!pctItem)
    {
        pctItem = new QTableWidgetItem();
        pctItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(row, 2 + dateCols + 1, pctItem);
    }
    pctItem->setText(QString::number(absentPct, 'f', 1) + "%");
    QFont pf = pctItem->font();
    pf.setBold(true);
    pctItem->setFont(pf);

    if (absentPct > 25.0)
        pctItem->setForeground(QColor(220, 20, 60));
    else if (absentPct > 10.0)
        pctItem->setForeground(QColor(255, 165, 0));
    else
        pctItem->setForeground(QColor(34, 168, 90));
}

void TeacherAttendanceDialog::updateStatsLabel()
{
    if (students.empty() || allDates.isEmpty())
    {
        lblStats->setText("Students: 0");
        return;
    }

    // Compute total occurrences of low attendance
    QVector<QString> visDates = visibleDates();
    int lowCount = 0;
    for (int i = 0; i < students.size(); ++i)
    {
        int sid = students[i]->getId();
        int totalAbsent = 0;
        for (const QString &d : allDates)
        {
            int visIdx = visDates.indexOf(d);
            if (visIdx >= 0)
            {
                QTableWidgetItem *c = table->item(i, 2 + visIdx);
                if (c && !c->data(Qt::UserRole + 2).toBool())
                    totalAbsent++;
            }
            else
            {
                if (!currentPresenceSet.contains(QString::number(sid) + "_" + d))
                    totalAbsent++;
            }
        }
        double attendPct = allDates.isEmpty() ? 100.0 : (1.0 - (double)totalAbsent / allDates.size()) * 100.0;
        if (attendPct < 75.0)
            lowCount++;
    }

    lblStats->setText(
        QString("Students: %1  |  Below 75%: %2")
            .arg(students.size())
            .arg(lowCount));

    if (lowCount > 0)
        lblStats->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #dc143c;").arg(AppFonts::Small));
    else
        lblStats->setStyleSheet(QString("font-size: %1px; font-weight: bold; color: #22a85a;").arg(AppFonts::Small));
}

void TeacherAttendanceDialog::onCourseChanged(int index)
{
    students.clear();
    allDates.clear();
    weekOffset = 0;

    if (index < 0 || comboCourse->count() == 0)
        return;

    currentCourseId = comboCourse->currentData().toInt();
    if (currentCourseId <= 0)
        return;

    auto c = myManager->getCourse(currentCourseId);
    if (!c)
        return;

    students = myManager->getStudentsByEnrollment(currentCourseId);

    // Fallback to general semester filtering when no explicit enrollments exist
    if (students.empty())
        students = myManager->getStudentsBySemester(c->getSemester());

    allDates = myManager->getCourseDates(currentCourseId);
    std::sort(allDates.begin(), allDates.end());
    currentPresenceSet = myManager->getPresenceSet(currentCourseId);

    buildTable();
}

void TeacherAttendanceDialog::onCellClicked(int row, int col)
{
    // Prevent actions on non-editable indicator columns
    int dateCols = visibleDateCount();
    if (col < 2 || col >= 2 + dateCols)
        return;

    QTableWidgetItem *cell = table->item(row, col);
    if (!cell)
        return;

    bool wasPresent = cell->data(Qt::UserRole + 2).toBool();
    bool nowPresent = !wasPresent;

    colorCell(row, col, nowPresent);
    selectedDateCol = col;

    recalcRowStats(row);
    updateStatsLabel();
}

void TeacherAttendanceDialog::onHeaderClicked(int logicalIndex)
{
    int dateCols = visibleDateCount();
    if (logicalIndex >= 2 && logicalIndex < 2 + dateCols)
    {
        selectedDateCol = logicalIndex;
        // Visually highlight the target column
        for (int i = 0; i < table->rowCount(); ++i)
        {
            QTableWidgetItem *cell = table->item(i, logicalIndex);
            if (cell)
            {
                // Retain existing indicator color and outline item selection
                cell->setSelected(true);
            }
        }
    }
}

void TeacherAttendanceDialog::onTableContextMenu(const QPoint &pos)
{
    QTableWidgetItem *item = table->itemAt(pos);
    if (!item)
        return;

    int col = item->column();
    int dateCols = visibleDateCount();
    if (col < 2 || col >= 2 + dateCols)
        return;

    selectedDateCol = col;
    QVector<QString> visDates = visibleDates();
    int dateIdx = col - 2;
    QString dateStr = visDates[dateIdx];
    QDate dt = QDate::fromString(dateStr, "yyyy-MM-dd");

    QMenu menu(this);
    menu.setTitle(dt.toString("dd MMM yyyy (ddd)"));

    QAction *actColPresent = menu.addAction("Mark entire column Present");
    QAction *actColAbsent = menu.addAction("Mark entire column Absent");
    menu.addSeparator();
    QAction *actAllPresent = menu.addAction("Mark ALL visible Present");
    QAction *actAllAbsent = menu.addAction("Mark ALL visible Absent");

    QAction *chosen = menu.exec(table->viewport()->mapToGlobal(pos));
    if (chosen == actColPresent)
        onMarkColumnPresentClicked();
    else if (chosen == actColAbsent)
        onMarkColumnAbsentClicked();
    else if (chosen == actAllPresent)
        onMarkAllPresentClicked();
    else if (chosen == actAllAbsent)
        onMarkAllAbsentClicked();
}

void TeacherAttendanceDialog::onAddTodayClicked()
{
    if (currentCourseId <= 0)
        return;

    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    if (allDates.contains(today))
    {
        QMessageBox::information(this, "Already Exists", "Today's date is already added.");
        return;
    }

    // Default to absent when instantiating new class columns
    for (const auto &s : students)
        myManager->markAttendance(currentCourseId, s->getId(), today, false);

    allDates.append(today);
    std::sort(allDates.begin(), allDates.end());

    // Snap view to the end of the timeline
    weekOffset = 0;
    buildTable();

    QMessageBox::information(this, "Date Added",
                             "Added class date: " + today + "\nAll students marked Absent by default.\nClick cells to mark Present.");
}

void TeacherAttendanceDialog::onMarkAllPresentClicked()
{
    int dateCols = visibleDateCount();
    if (dateCols == 0)
        return;

    for (int i = 0; i < table->rowCount(); ++i)
    {
        for (int j = 0; j < dateCols; ++j)
            colorCell(i, 2 + j, true);
        recalcRowStats(i);
    }
    updateStatsLabel();
}

void TeacherAttendanceDialog::onMarkAllAbsentClicked()
{
    int dateCols = visibleDateCount();
    if (dateCols == 0)
        return;

    for (int i = 0; i < table->rowCount(); ++i)
    {
        for (int j = 0; j < dateCols; ++j)
            colorCell(i, 2 + j, false);
        recalcRowStats(i);
    }
    updateStatsLabel();
}

void TeacherAttendanceDialog::onMarkColumnPresentClicked()
{
    int dateCols = visibleDateCount();
    if (selectedDateCol < 2 || selectedDateCol >= 2 + dateCols)
    {
        QMessageBox::information(this, "Select Column",
                                 "Click on a date column header first, then use this button.");
        return;
    }

    for (int i = 0; i < table->rowCount(); ++i)
    {
        colorCell(i, selectedDateCol, true);
        recalcRowStats(i);
    }
    updateStatsLabel();
}

void TeacherAttendanceDialog::onMarkColumnAbsentClicked()
{
    int dateCols = visibleDateCount();
    if (selectedDateCol < 2 || selectedDateCol >= 2 + dateCols)
    {
        QMessageBox::information(this, "Select Column",
                                 "Click on a date column header first, then use this button.");
        return;
    }

    for (int i = 0; i < table->rowCount(); ++i)
    {
        colorCell(i, selectedDateCol, false);
        recalcRowStats(i);
    }
    updateStatsLabel();
}

void TeacherAttendanceDialog::onSaveClicked()
{
    if (currentCourseId <= 0)
        return;

    QVector<QString> visDates = visibleDates();
    int dateCols = visDates.size();
    QVector<ManagerAcademics::AttendanceUpdate> attUpdates;

    for (int i = 0; i < table->rowCount(); ++i)
    {
        int sid = table->item(i, 0)->text().toInt();
        for (int j = 0; j < dateCols; ++j)
        {
            QTableWidgetItem *cell = table->item(i, 2 + j);
            if (!cell)
                continue;
            bool present = cell->data(Qt::UserRole + 2).toBool();
            attUpdates.append({sid, visDates[j], present});

            // Manually refresh local memory structure to ensure cross-page cohesion
            QString key = QString::number(sid) + "_" + visDates[j];
            if (present)
                currentPresenceSet.insert(key);
            else
                currentPresenceSet.remove(key);
        }
    }

    if (!attUpdates.isEmpty())
    {
        auto batch = std::make_shared<BatchMarkAttendanceCommand>(currentCourseId, attUpdates);
        myManager->executeCommand(batch);
    }
    QMessageBox::information(this, "Saved", "Attendance saved successfully!");
}

void TeacherAttendanceDialog::onExportClicked()
{
    if (currentCourseId <= 0 || students.empty())
        return;

    QString courseName = comboCourse->currentText().replace(" ", "_");
    QString defaultName = "attendance_" + courseName + ".csv";
    QString path = QFileDialog::getSaveFileName(this, "Export Attendance", defaultName, "CSV Files (*.csv)");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    // Dump metadata summary preamble
    out << "Course:," << comboCourse->currentText() << "\n";
    out << "Number of Classes Held:," << allDates.size() << "\n";
    out << "Total Students:," << students.size() << "\n\n";

    // Print CSV fields legend
    out << "ID,Name";
    for (const QString &d : allDates)
        out << "," << d;
    out << ",Total Present,Total Absent,Absent %,Attendance %\n";

    // Aggregate student performance lines
    QVector<QString> visDates = visibleDates();
    for (int i = 0; i < students.size(); ++i)
    {
        int sid = students[i]->getId();
        out << sid << "," << students[i]->getName().trimmed();

        int presentCount = 0;
        for (const QString &d : allDates)
        {
            int visIdx = visDates.indexOf(d);
            bool present;
            if (visIdx >= 0)
            {
                QTableWidgetItem *c = table->item(i, 2 + visIdx);
                present = c ? c->data(Qt::UserRole + 2).toBool() : false;
            }
            else
            {
                present = currentPresenceSet.contains(QString::number(sid) + "_" + d);
            }

            out << "," << (present ? "P" : "A");
            if (present)
                presentCount++;
        }

        int absentCount = allDates.size() - presentCount;
        double absentPct = allDates.isEmpty() ? 0.0 : (double)absentCount / allDates.size() * 100.0;
        double attendPct = 100.0 - absentPct;

        out << "," << presentCount
            << "," << absentCount
            << "," << QString::number(absentPct, 'f', 1) << "%"
            << "," << QString::number(attendPct, 'f', 1) << "%\n";
    }

    file.close();
    QMessageBox::information(this, "Exported", "Attendance exported to:\n" + path);
}

void TeacherAttendanceDialog::onWeekPrev()
{
    weekOffset++;
    buildTable();
}

void TeacherAttendanceDialog::onWeekNext()
{
    if (weekOffset > 0)
    {
        weekOffset--;
        buildTable();
    }
}
