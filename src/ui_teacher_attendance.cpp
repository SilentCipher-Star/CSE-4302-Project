#include "../include/ui_teacher_attendance.hpp"
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

TeacherAttendanceDialog::TeacherAttendanceDialog(AcadenceManager *manager, int teacherId, QWidget *parent)
    : QDialog(parent), myManager(manager), teacherId(teacherId)
{
    setWindowTitle("Daily Attendance Tracker");
    setMinimumSize(950, 600);
    resize(1150, 680);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ── Title ──
    QLabel *title = new QLabel("Daily Attendance Sheet");
    title->setStyleSheet("font-size: 18px; font-weight: bold; padding: 4px 0;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // ── Top bar: course selector + classes held ──
    QHBoxLayout *topBar = new QHBoxLayout();

    topBar->addWidget(new QLabel("Course:"));
    comboCourse = new QComboBox();
    comboCourse->setMinimumWidth(260);
    topBar->addWidget(comboCourse);

    topBar->addSpacing(20);
    lblClassesHeld = new QLabel("Number of Classes Held: 0");
    lblClassesHeld->setStyleSheet("font-size: 13px; font-weight: bold; color: #1a73e8;");
    topBar->addWidget(lblClassesHeld);

    topBar->addStretch();

    lblStats = new QLabel();
    lblStats->setStyleSheet("font-size: 12px; font-weight: bold;");
    topBar->addWidget(lblStats);

    mainLayout->addLayout(topBar);

    // ── Week navigation ──
    QHBoxLayout *navBar = new QHBoxLayout();

    btnPrevWeek = new QPushButton("<<  Prev Week");
    btnPrevWeek->setFixedWidth(130);
    btnPrevWeek->setStyleSheet("padding: 5px 10px;");
    navBar->addWidget(btnPrevWeek);

    lblWeekRange = new QLabel();
    lblWeekRange->setAlignment(Qt::AlignCenter);
    lblWeekRange->setStyleSheet("font-size: 13px; font-weight: bold;");
    navBar->addWidget(lblWeekRange, 1);

    btnNextWeek = new QPushButton("Next Week  >>");
    btnNextWeek->setFixedWidth(130);
    btnNextWeek->setStyleSheet("padding: 5px 10px;");
    navBar->addWidget(btnNextWeek);

    mainLayout->addLayout(navBar);

    // ── Separator ──
    QFrame *sep = new QFrame();
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("color: #444;");
    mainLayout->addWidget(sep);

    // ── Action buttons row ──
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

    // ── Legend ──
    QHBoxLayout *legendRow = new QHBoxLayout();
    auto addLegend = [&](const QString &bgColor, const QString &text)
    {
        QLabel *box = new QLabel();
        box->setFixedSize(18, 18);
        box->setStyleSheet(QString("background-color: %1; border: 1px solid #555; border-radius: 2px;").arg(bgColor));
        legendRow->addWidget(box);
        QLabel *lbl = new QLabel(text);
        lbl->setStyleSheet("font-size: 11px;");
        legendRow->addWidget(lbl);
        legendRow->addSpacing(14);
    };
    addLegend("#22a85a", "Present (P)");
    addLegend("#dc143c", "Absent (A)");
    addLegend("#555", "No Class");
    legendRow->addStretch();
    QLabel *hint = new QLabel("Click cells to toggle | Click column headers for column operations");
    hint->setStyleSheet("font-size: 10px; color: #888;");
    legendRow->addWidget(hint);
    mainLayout->addLayout(legendRow);

    // ── Table ──
    table = new QTableWidget();
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    table->setStyleSheet(
        "QTableWidget { gridline-color: #555; font-size: 12px; }"
        "QTableWidget::item { padding: 4px; }"
        "QHeaderView::section { font-weight: bold; padding: 4px; }");
    mainLayout->addWidget(table, 1);

    // ── Bottom bar ──
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

    // ── Populate courses ──
    QVector<Course *> courses = myManager->getTeacherCourses(teacherId);
    for (auto *c : courses)
    {
        comboCourse->addItem(c->getCode() + " - " + c->getName(), c->getId());
    }
    qDeleteAll(courses);

    // ── Connections ──
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

    // ── Initial load ──
    if (comboCourse->count() > 0)
        onCourseChanged(0);
}

TeacherAttendanceDialog::~TeacherAttendanceDialog()
{
    qDeleteAll(students);
}

// ─── Helpers ──────────────────────────────────────────────────

int TeacherAttendanceDialog::visibleDateCount() const
{
    int start = (int)allDates.size() - datesPerPage - weekOffset * datesPerPage;
    if (start < 0) start = 0;
    int end = start + datesPerPage;
    if (end > allDates.size()) end = allDates.size();
    return end - start;
}

QVector<QString> TeacherAttendanceDialog::visibleDates() const
{
    if (allDates.isEmpty()) return {};

    int total = allDates.size();
    int start = total - datesPerPage - weekOffset * datesPerPage;
    if (start < 0) start = 0;
    int end = start + datesPerPage;
    if (end > total) end = total;

    QVector<QString> result;
    for (int i = start; i < end; ++i)
        result.append(allDates[i]);
    return result;
}

// ─── Build the spreadsheet table ─────────────────────────────

void TeacherAttendanceDialog::buildTable()
{
    table->clear();
    table->setRowCount(0);
    table->setColumnCount(0);
    selectedDateCol = -1;

    if (currentCourseId <= 0) return;

    QVector<QString> dates = visibleDates();
    int dateCols = dates.size();
    int totalCols = 2 + dateCols + 2;  // ID, Name, [dates...], Absent, Absent%

    QStringList headers;
    headers << "ID" << "Student Name";
    for (const QString &d : dates)
    {
        QDate dt = QDate::fromString(d, "yyyy-MM-dd");
        headers << dt.toString("dd MMM\nddd");  // e.g. "10 Mar\nTue"
    }
    headers << "Absent" << "Absent %";

    table->setColumnCount(totalCols);
    table->setHorizontalHeaderLabels(headers);
    table->setRowCount(students.size());

    // Header row height for multi-line date headers
    table->horizontalHeader()->setMinimumHeight(40);

    // Set column widths
    table->setColumnWidth(0, 50);   // ID
    table->setColumnWidth(1, 160);  // Name
    for (int j = 0; j < dateCols; ++j)
        table->setColumnWidth(2 + j, 72);
    table->setColumnWidth(2 + dateCols, 65);      // Absent
    table->setColumnWidth(2 + dateCols + 1, 85);  // Absent %

    for (int i = 0; i < students.size(); ++i)
    {
        int sid = students[i]->getId();
        QString name = students[i]->getName().trimmed();

        // ID column
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(sid));
        idItem->setTextAlignment(Qt::AlignCenter);
        table->setItem(i, 0, idItem);

        // Name column
        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        table->setItem(i, 1, nameItem);

        // Visible date columns with clickable P/A cells
        for (int j = 0; j < dateCols; ++j)
        {
            bool present = myManager->isPresent(currentCourseId, sid, dates[j]);

            QTableWidgetItem *cell = new QTableWidgetItem(present ? "P" : "A");
            cell->setTextAlignment(Qt::AlignCenter);
            cell->setData(Qt::UserRole, dates[j]);         // store date
            cell->setData(Qt::UserRole + 1, sid);           // store student id
            cell->setData(Qt::UserRole + 2, present);       // store status

            QFont f = cell->font();
            f.setBold(true);
            cell->setFont(f);

            table->setItem(i, 2 + j, cell);
            colorCell(i, 2 + j, present);
        }

        // Calculate absent stats across ALL dates (not just visible)
        recalcRowStats(i);
    }

    table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    updateStatsLabel();

    // Update week navigation label
    if (!dates.isEmpty())
    {
        QDate first = QDate::fromString(dates.first(), "yyyy-MM-dd");
        QDate last  = QDate::fromString(dates.last(), "yyyy-MM-dd");
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

    // Enable/disable navigation buttons
    int maxOffset = allDates.isEmpty() ? 0 : (allDates.size() - 1) / datesPerPage;
    btnPrevWeek->setEnabled(weekOffset < maxOffset);
    btnNextWeek->setEnabled(weekOffset > 0);

    lblClassesHeld->setText(QString("Number of Classes Held: %1").arg(allDates.size()));
}

void TeacherAttendanceDialog::colorCell(int row, int col, bool present)
{
    QTableWidgetItem *cell = table->item(row, col);
    if (!cell) return;

    if (present)
    {
        cell->setText("P");
        cell->setBackground(QColor(34, 168, 90, 80));
        cell->setForeground(QColor(20, 120, 50));
    }
    else
    {
        cell->setText("A");
        cell->setBackground(QColor(220, 20, 60, 80));
        cell->setForeground(QColor(180, 10, 40));
    }

    cell->setData(Qt::UserRole + 2, present);
}

void TeacherAttendanceDialog::recalcRowStats(int row)
{
    if (row < 0 || row >= students.size()) return;

    int sid = students[row]->getId();
    int dateCols = visibleDateCount();
    QVector<QString> visDates = visibleDates();

    int totalAbsent = 0;
    for (const QString &d : allDates)
    {
        int visIdx = visDates.indexOf(d);
        if (visIdx >= 0)
        {
            // Use the table cell's current state
            QTableWidgetItem *c = table->item(row, 2 + visIdx);
            if (c && !c->data(Qt::UserRole + 2).toBool())
                totalAbsent++;
        }
        else
        {
            // Use saved data
            if (!myManager->isPresent(currentCourseId, sid, d))
                totalAbsent++;
        }
    }

    // Absent count column
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

    // Absent % column
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
    if (students.isEmpty() || allDates.isEmpty())
    {
        lblStats->setText("Students: 0");
        return;
    }

    // Count students below 75% attendance
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
                if (!myManager->isPresent(currentCourseId, sid, d))
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
        lblStats->setStyleSheet("font-size: 12px; font-weight: bold; color: #dc143c;");
    else
        lblStats->setStyleSheet("font-size: 12px; font-weight: bold; color: #22a85a;");
}

// ─── Slots ────────────────────────────────────────────────────

void TeacherAttendanceDialog::onCourseChanged(int index)
{
    qDeleteAll(students);
    students.clear();
    allDates.clear();
    weekOffset = 0;

    if (index < 0 || comboCourse->count() == 0) return;

    currentCourseId = comboCourse->currentData().toInt();
    if (currentCourseId <= 0) return;

    std::unique_ptr<Course> c(myManager->getCourse(currentCourseId));
    if (!c) return;

    students = myManager->getStudentsByEnrollment(currentCourseId);

    // If no enrollments, fall back to semester-based students
    if (students.isEmpty())
        students = myManager->getStudentsBySemester(c->getSemester());

    allDates = myManager->getCourseDates(currentCourseId);
    std::sort(allDates.begin(), allDates.end());

    buildTable();
}

void TeacherAttendanceDialog::onCellClicked(int row, int col)
{
    // Only respond to clicks on date columns
    int dateCols = visibleDateCount();
    if (col < 2 || col >= 2 + dateCols) return;

    QTableWidgetItem *cell = table->item(row, col);
    if (!cell) return;

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
        // Visual feedback - briefly highlight the column
        for (int i = 0; i < table->rowCount(); ++i)
        {
            QTableWidgetItem *cell = table->item(i, logicalIndex);
            if (cell)
            {
                // Keep existing color but add selection indicator in the border
                cell->setSelected(true);
            }
        }
    }
}

void TeacherAttendanceDialog::onTableContextMenu(const QPoint &pos)
{
    QTableWidgetItem *item = table->itemAt(pos);
    if (!item) return;

    int col = item->column();
    int dateCols = visibleDateCount();
    if (col < 2 || col >= 2 + dateCols) return;

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
    if (currentCourseId <= 0) return;

    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    if (allDates.contains(today))
    {
        QMessageBox::information(this, "Already Exists", "Today's date is already added.");
        return;
    }

    // Mark all students absent initially for today
    for (auto *s : students)
        myManager->markAttendance(currentCourseId, s->getId(), today, false);

    allDates.append(today);
    std::sort(allDates.begin(), allDates.end());

    // Jump to latest page
    weekOffset = 0;
    buildTable();

    QMessageBox::information(this, "Date Added",
        "Added class date: " + today + "\nAll students marked Absent by default.\nClick cells to mark Present.");
}

void TeacherAttendanceDialog::onMarkAllPresentClicked()
{
    int dateCols = visibleDateCount();
    if (dateCols == 0) return;

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
    if (dateCols == 0) return;

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
    if (currentCourseId <= 0) return;

    QVector<QString> visDates = visibleDates();
    int dateCols = visDates.size();

    for (int i = 0; i < table->rowCount(); ++i)
    {
        int sid = table->item(i, 0)->text().toInt();
        for (int j = 0; j < dateCols; ++j)
        {
            QTableWidgetItem *cell = table->item(i, 2 + j);
            if (!cell) continue;
            bool present = cell->data(Qt::UserRole + 2).toBool();
            myManager->markAttendance(currentCourseId, sid, visDates[j], present);
        }
    }

    QMessageBox::information(this, "Saved", "Attendance saved successfully!");
}

void TeacherAttendanceDialog::onExportClicked()
{
    if (currentCourseId <= 0 || students.isEmpty()) return;

    QString courseName = comboCourse->currentText().replace(" ", "_");
    QString defaultName = "attendance_" + courseName + ".csv";
    QString path = QFileDialog::getSaveFileName(this, "Export Attendance", defaultName, "CSV Files (*.csv)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    // Course info header
    out << "Course:," << comboCourse->currentText() << "\n";
    out << "Number of Classes Held:," << allDates.size() << "\n";
    out << "Total Students:," << students.size() << "\n\n";

    // Column header
    out << "ID,Name";
    for (const QString &d : allDates)
        out << "," << d;
    out << ",Total Present,Total Absent,Absent %,Attendance %\n";

    // Data rows
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
                present = myManager->isPresent(currentCourseId, sid, d);
            }

            out << "," << (present ? "P" : "A");
            if (present) presentCount++;
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
