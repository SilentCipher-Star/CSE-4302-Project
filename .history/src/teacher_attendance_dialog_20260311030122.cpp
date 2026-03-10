#include "../include/teacher_attendance_dialog.hpp"
#include "../include/manager_academics.hpp"
#include "../include/manager_persons.hpp"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QMenu>
#include <QScrollBar>
#include <algorithm>

TeacherAttendanceDialog::TeacherAttendanceDialog(AcadenceManager *manager, int teacherId, QWidget *parent)
    : QDialog(parent), m_manager(manager), m_teacherId(teacherId)
{
    // Start on current week's Monday
    QDate today = QDate::currentDate();
    int dayOfWeek = today.dayOfWeek(); // 1=Mon, 7=Sun
    m_currentWeekStart = today.addDays(-(dayOfWeek - 1));

    setWindowTitle("Attendance Sheet");
    setMinimumSize(900, 600);
    buildUI();

    // Populate courses
    QVector<Course *> courses = m_manager->getTeacherCourses(m_teacherId);
    for (auto c : courses)
    {
        m_comboCourse->addItem(c->getCode() + " - " + c->getName(), c->getId());
    }
    qDeleteAll(courses);

    if (m_comboCourse->count() > 0)
        onCourseChanged(0);
}

void TeacherAttendanceDialog::buildUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);
    mainLayout->setContentsMargins(12, 12, 12, 12);

    // ── Title ──
    QLabel *title = new QLabel("Daily Attendance Sheet");
    title->setStyleSheet("font-size: 18px; font-weight: bold; padding: 4px 0;");
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // ── Course selector row ──
    QHBoxLayout *courseRow = new QHBoxLayout();
    courseRow->addWidget(new QLabel("Course:"));
    m_comboCourse = new QComboBox();
    m_comboCourse->setMinimumWidth(250);
    courseRow->addWidget(m_comboCourse);
    courseRow->addStretch();

    m_lblClassesHeld = new QLabel("Classes Held: 0");
    m_lblClassesHeld->setStyleSheet("font-size: 13px; font-weight: bold; color: #1a73e8;");
    courseRow->addWidget(m_lblClassesHeld);
    mainLayout->addLayout(courseRow);

    // ── Week navigation row ──
    QHBoxLayout *weekRow = new QHBoxLayout();
    m_btnPrevWeek = new QPushButton("<< Prev Week");
    m_btnPrevWeek->setFixedWidth(120);
    weekRow->addWidget(m_btnPrevWeek);

    m_lblWeek = new QLabel();
    m_lblWeek->setAlignment(Qt::AlignCenter);
    m_lblWeek->setStyleSheet("font-size: 14px; font-weight: bold;");
    weekRow->addWidget(m_lblWeek, 1);

    m_btnNextWeek = new QPushButton("Next Week >>");
    m_btnNextWeek->setFixedWidth(120);
    weekRow->addWidget(m_btnNextWeek);
    mainLayout->addLayout(weekRow);

    // ── Bulk action buttons ──
    QHBoxLayout *actionRow = new QHBoxLayout();

    m_btnAddDate = new QPushButton("+ Add Today's Class");
    m_btnAddDate->setStyleSheet("background-color: #1a73e8; color: white; padding: 5px 12px; border-radius: 4px;");
    actionRow->addWidget(m_btnAddDate);

    m_btnMarkAllPresent = new QPushButton("Mark All Present");
    m_btnMarkAllPresent->setStyleSheet("background-color: #22a85a; color: white; padding: 5px 12px; border-radius: 4px;");
    actionRow->addWidget(m_btnMarkAllPresent);

    m_btnMarkAllAbsent = new QPushButton("Mark All Absent");
    m_btnMarkAllAbsent->setStyleSheet("background-color: #dc143c; color: white; padding: 5px 12px; border-radius: 4px;");
    actionRow->addWidget(m_btnMarkAllAbsent);

    m_btnMarkColPresent = new QPushButton("Column -> Present");
    m_btnMarkColPresent->setToolTip("Mark all students Present for the selected date column");
    actionRow->addWidget(m_btnMarkColPresent);

    m_btnMarkColAbsent = new QPushButton("Column -> Absent");
    m_btnMarkColAbsent->setToolTip("Mark all students Absent for the selected date column");
    actionRow->addWidget(m_btnMarkColAbsent);

    actionRow->addStretch();
    mainLayout->addLayout(actionRow);

    // ── Legend ──
    QHBoxLayout *legendRow = new QHBoxLayout();
    auto addLegend = [&](const QString &color, const QString &label)
    {
        QLabel *box = new QLabel();
        box->setFixedSize(16, 16);
        box->setStyleSheet(QString("background-color: %1; border: 1px solid #555; border-radius: 2px;").arg(color));
        legendRow->addWidget(box);
        legendRow->addWidget(new QLabel(label));
        legendRow->addSpacing(12);
    };
    addLegend("#22a85a", "Present (P)");
    addLegend("#dc143c", "Absent (A)");
    addLegend("#555555", "No Class");
    legendRow->addStretch();
    mainLayout->addLayout(legendRow);

    // ── Attendance Table ──
    m_table = new QTableWidget();
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setStretchLastSection(false);
    m_table->verticalHeader()->setVisible(false);
    m_table->setAlternatingRowColors(true);
    m_table->setStyleSheet(
        "QTableWidget { gridline-color: #444; font-size: 12px; }"
        "QTableWidget::item { padding: 4px; }"
        "QHeaderView::section { padding: 4px; font-weight: bold; }");
    mainLayout->addWidget(m_table, 1);

    // ── Stats row ──
    m_lblStats = new QLabel();
    m_lblStats->setStyleSheet("font-size: 12px; padding: 4px;");
    mainLayout->addWidget(m_lblStats);

    // ── Bottom buttons ──
    QHBoxLayout *bottomRow = new QHBoxLayout();

    m_btnExport = new QPushButton("Export to CSV");
    m_btnExport->setFixedWidth(140);
    bottomRow->addWidget(m_btnExport);

    bottomRow->addStretch();

    m_btnSave = new QPushButton("Save Attendance");
    m_btnSave->setStyleSheet("background-color: #22a85a; color: white; padding: 6px 20px; border-radius: 4px; font-weight: bold;");
    bottomRow->addWidget(m_btnSave);

    m_btnClose = new QPushButton("Close");
    m_btnClose->setFixedWidth(100);
    bottomRow->addWidget(m_btnClose);

    mainLayout->addLayout(bottomRow);

    // ── Connections ──
    connect(m_comboCourse, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TeacherAttendanceDialog::onCourseChanged);
    connect(m_btnPrevWeek, &QPushButton::clicked, this, &TeacherAttendanceDialog::onPrevWeek);
    connect(m_btnNextWeek, &QPushButton::clicked, this, &TeacherAttendanceDialog::onNextWeek);
    connect(m_table, &QTableWidget::cellClicked, this, &TeacherAttendanceDialog::onCellClicked);
    connect(m_btnMarkAllPresent, &QPushButton::clicked, this, &TeacherAttendanceDialog::onMarkAllPresent);
    connect(m_btnMarkAllAbsent, &QPushButton::clicked, this, &TeacherAttendanceDialog::onMarkAllAbsent);
    connect(m_btnMarkColPresent, &QPushButton::clicked, this, &TeacherAttendanceDialog::onMarkColumnPresent);
    connect(m_btnMarkColAbsent, &QPushButton::clicked, this, &TeacherAttendanceDialog::onMarkColumnAbsent);
    connect(m_btnAddDate, &QPushButton::clicked, this, &TeacherAttendanceDialog::onAddClassDate);
    connect(m_btnSave, &QPushButton::clicked, this, &TeacherAttendanceDialog::onSaveAttendance);
    connect(m_btnExport, &QPushButton::clicked, this, &TeacherAttendanceDialog::onExportAttendance);
    connect(m_btnClose, &QPushButton::clicked, this, &QDialog::accept);
}

void TeacherAttendanceDialog::onCourseChanged(int index)
{
    if (index < 0)
        return;

    int courseId = m_comboCourse->currentData().toInt();

    // Load students
    m_students.clear();
    std::unique_ptr<Course> c(m_manager->getCourse(courseId));
    if (!c)
        return;

    QVector<Student *> students = m_manager->getStudentsBySemester(c->getSemester());
    for (auto *s : students)
    {
        m_students.append({s->getId(), s->getName()});
    }
    qDeleteAll(students);

    // Sort students by name
    std::sort(m_students.begin(), m_students.end(), [](const StudentRow &a, const StudentRow &b)
              { return a.name.toLower() < b.name.toLower(); });

    // Load all dates
    m_allDates = m_manager->getCourseDates(courseId);
    std::sort(m_allDates.begin(), m_allDates.end());

    // Load attendance into cache
    m_attendanceCache.clear();
    for (const auto &stu : m_students)
    {
        for (const auto &date : m_allDates)
        {
            bool present = m_manager->isPresent(courseId, stu.studentId, date);
            m_attendanceCache[{stu.studentId, date}] = present;
        }
    }

    m_lblClassesHeld->setText(QString("Number of Classes Held: %1").arg(m_allDates.size()));

    refreshTable();
}

void TeacherAttendanceDialog::onWeekChanged()
{
    refreshTable();
}

void TeacherAttendanceDialog::onPrevWeek()
{
    m_currentWeekStart = m_currentWeekStart.addDays(-7);
    refreshTable();
}

void TeacherAttendanceDialog::onNextWeek()
{
    m_currentWeekStart = m_currentWeekStart.addDays(7);
    refreshTable();
}

QDate TeacherAttendanceDialog::getWeekStart() const
{
    return m_currentWeekStart;
}

QDate TeacherAttendanceDialog::getWeekEnd() const
{
    return m_currentWeekStart.addDays(6);
}

void TeacherAttendanceDialog::updateWeekLabel()
{
    QDate start = getWeekStart();
    QDate end = getWeekEnd();
    m_lblWeek->setText(QString("Week: %1 to %2")
                           .arg(start.toString("dd MMM yyyy"))
                           .arg(end.toString("dd MMM yyyy")));
}

void TeacherAttendanceDialog::refreshTable()
{
    updateWeekLabel();
    m_table->clear();
    m_table->setRowCount(0);
    m_selectedColumn = -1;

    // Find dates that fall within the current week
    m_weekDates.clear();
    QDate start = getWeekStart();
    QDate end = getWeekEnd();

    for (const auto &dateStr : m_allDates)
    {
        QDate d = QDate::fromString(dateStr, "yyyy-MM-dd");
        if (d.isValid() && d >= start && d <= end)
        {
            m_weekDates.append(dateStr);
        }
    }

    // Build headers: ID | Name | [Date columns...] | Total | Absent | Absent%
    int fixedColsBefore = 2; // ID, Name
    int fixedColsAfter = 3;  // Total Present, Total Absent, Absent%
    int totalCols = fixedColsBefore + m_weekDates.size() + fixedColsAfter;

    m_table->setColumnCount(totalCols);

    QStringList headers;
    headers << "ID" << "Student Name";
    for (const auto &dateStr : m_weekDates)
    {
        QDate d = QDate::fromString(dateStr, "yyyy-MM-dd");
        headers << d.toString("dd/MM\nddd"); // e.g. "10/02\nMon"
    }
    headers << "Present" << "Absent" << "Absent %";
    m_table->setHorizontalHeaderLabels(headers);

    // Set column widths
    m_table->setColumnWidth(0, 50);
    m_table->setColumnWidth(1, 160);
    for (int j = 0; j < m_weekDates.size(); ++j)
        m_table->setColumnWidth(fixedColsBefore + j, 65);

    int afterStart = fixedColsBefore + m_weekDates.size();
    m_table->setColumnWidth(afterStart, 70);
    m_table->setColumnWidth(afterStart + 1, 70);
    m_table->setColumnWidth(afterStart + 2, 80);

    m_table->setRowCount(m_students.size());

    for (int i = 0; i < m_students.size(); ++i)
    {
        const auto &stu = m_students[i];

        // ID column
        QTableWidgetItem *idItem = new QTableWidgetItem(QString::number(stu.studentId));
        idItem->setTextAlignment(Qt::AlignCenter);
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 0, idItem);

        // Name column
        QTableWidgetItem *nameItem = new QTableWidgetItem(stu.name);
        nameItem->setFlags(nameItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, 1, nameItem);

        // Date columns
        int presentCount = 0;
        int totalClassesForStudent = m_allDates.size();

        // Count total presents across ALL dates (not just this week)
        for (const auto &dateStr : m_allDates)
        {
            auto key = QPair<int, QString>(stu.studentId, dateStr);
            if (m_attendanceCache.value(key, false))
                presentCount++;
        }

        // Show this week's date cells
        for (int j = 0; j < m_weekDates.size(); ++j)
        {
            auto key = QPair<int, QString>(stu.studentId, m_weekDates[j]);
            bool present = m_attendanceCache.value(key, false);

            QTableWidgetItem *item = new QTableWidgetItem(present ? "P" : "A");
            item->setTextAlignment(Qt::AlignCenter);
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            colorCell(i, fixedColsBefore + j, present);
            m_table->setItem(i, fixedColsBefore + j, item);
        }

        // Summary columns
        int absentCount = totalClassesForStudent - presentCount;
        double absentPct = (totalClassesForStudent > 0)
                               ? (double)absentCount / totalClassesForStudent * 100.0
                               : 0.0;

        QTableWidgetItem *presentItem = new QTableWidgetItem(QString::number(presentCount));
        presentItem->setTextAlignment(Qt::AlignCenter);
        presentItem->setFlags(presentItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(i, afterStart, presentItem);

        QTableWidgetItem *absentItem = new QTableWidgetItem(QString::number(absentCount));
        absentItem->setTextAlignment(Qt::AlignCenter);
        absentItem->setFlags(absentItem->flags() & ~Qt::ItemIsEditable);
        if (absentCount > 0)
            absentItem->setForeground(QColor(220, 20, 60));
        m_table->setItem(i, afterStart + 1, absentItem);

        QTableWidgetItem *pctItem = new QTableWidgetItem(QString::number(absentPct, 'f', 1) + "%");
        pctItem->setTextAlignment(Qt::AlignCenter);
        pctItem->setFlags(pctItem->flags() & ~Qt::ItemIsEditable);
        if (absentPct > 25.0)
            pctItem->setForeground(QColor(220, 20, 60));
        else if (absentPct > 15.0)
            pctItem->setForeground(QColor(255, 165, 0));
        else
            pctItem->setForeground(QColor(34, 139, 34));
        m_table->setItem(i, afterStart + 2, pctItem);
    }

    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

    updateSummaryStats();
}

void TeacherAttendanceDialog::colorCell(int row, int col, bool present)
{
    QTableWidgetItem *item = m_table->item(row, col);
    if (!item)
        return;

    if (present)
    {
        item->setBackground(QColor(34, 168, 90, 180));
        item->setForeground(QColor(255, 255, 255));
        item->setText("P");
    }
    else
    {
        item->setBackground(QColor(220, 20, 60, 180));
        item->setForeground(QColor(255, 255, 255));
        item->setText("A");
    }
}

void TeacherAttendanceDialog::onCellClicked(int row, int col)
{
    int fixedColsBefore = 2;
    int dateIndex = col - fixedColsBefore;

    // Track selected column for column-level operations
    if (dateIndex >= 0 && dateIndex < m_weekDates.size())
    {
        m_selectedColumn = col;
    }

    // Only toggle if it's a date column
    if (dateIndex < 0 || dateIndex >= m_weekDates.size())
        return;

    const QString &dateStr = m_weekDates[dateIndex];
    int studentId = m_students[row].studentId;
    auto key = QPair<int, QString>(studentId, dateStr);

    bool current = m_attendanceCache.value(key, false);
    m_attendanceCache[key] = !current;

    colorCell(row, col, !current);
    refreshTable(); // to update summary columns
}

void TeacherAttendanceDialog::onMarkAllPresent()
{
    if (m_weekDates.isEmpty())
    {
        QMessageBox::information(this, "No Dates", "No class dates in this week to mark.");
        return;
    }

    for (const auto &stu : m_students)
    {
        for (const auto &dateStr : m_weekDates)
        {
            m_attendanceCache[{stu.studentId, dateStr}] = true;
        }
    }
    refreshTable();
}

void TeacherAttendanceDialog::onMarkAllAbsent()
{
    if (m_weekDates.isEmpty())
    {
        QMessageBox::information(this, "No Dates", "No class dates in this week to mark.");
        return;
    }

    for (const auto &stu : m_students)
    {
        for (const auto &dateStr : m_weekDates)
        {
            m_attendanceCache[{stu.studentId, dateStr}] = false;
        }
    }
    refreshTable();
}

void TeacherAttendanceDialog::onMarkColumnPresent()
{
    int fixedColsBefore = 2;
    int dateIndex = m_selectedColumn - fixedColsBefore;

    if (dateIndex < 0 || dateIndex >= m_weekDates.size())
    {
        QMessageBox::information(this, "No Column", "Click on a date column first to select it.");
        return;
    }

    const QString &dateStr = m_weekDates[dateIndex];
    for (const auto &stu : m_students)
    {
        m_attendanceCache[{stu.studentId, dateStr}] = true;
    }
    refreshTable();
}

void TeacherAttendanceDialog::onMarkColumnAbsent()
{
    int fixedColsBefore = 2;
    int dateIndex = m_selectedColumn - fixedColsBefore;

    if (dateIndex < 0 || dateIndex >= m_weekDates.size())
    {
        QMessageBox::information(this, "No Column", "Click on a date column first to select it.");
        return;
    }

    const QString &dateStr = m_weekDates[dateIndex];
    for (const auto &stu : m_students)
    {
        m_attendanceCache[{stu.studentId, dateStr}] = false;
    }
    refreshTable();
}

void TeacherAttendanceDialog::onAddClassDate()
{
    int courseId = m_comboCourse->currentData().toInt();
    if (courseId <= 0)
        return;

    QString today = QDate::currentDate().toString("yyyy-MM-dd");

    // Check if today already exists
    if (m_allDates.contains(today))
    {
        QMessageBox::warning(this, "Duplicate", "Today's date already exists for this course.");
        return;
    }

    // Add date for all students (default absent)
    std::unique_ptr<Course> c(m_manager->getCourse(courseId));
    if (!c)
        return;

    QVector<Student *> students = m_manager->getStudentsBySemester(c->getSemester());
    for (auto s : students)
    {
        m_manager->markAttendance(courseId, s->getId(), today, false);
    }
    qDeleteAll(students);

    // Update cache
    m_allDates.append(today);
    std::sort(m_allDates.begin(), m_allDates.end());

    for (const auto &stu : m_students)
    {
        m_attendanceCache[{stu.studentId, today}] = false;
    }

    m_lblClassesHeld->setText(QString("Number of Classes Held: %1").arg(m_allDates.size()));

    // Navigate to the week containing today
    QDate todayDate = QDate::currentDate();
    int dayOfWeek = todayDate.dayOfWeek();
    m_currentWeekStart = todayDate.addDays(-(dayOfWeek - 1));

    refreshTable();
    QMessageBox::information(this, "Success", "Added class date: " + today);
}

void TeacherAttendanceDialog::onSaveAttendance()
{
    int courseId = m_comboCourse->currentData().toInt();
    if (courseId <= 0)
        return;

    // Save ALL cached attendance (not just this week)
    for (auto it = m_attendanceCache.constBegin(); it != m_attendanceCache.constEnd(); ++it)
    {
        int studentId = it.key().first;
        QString date = it.key().second;
        bool present = it.value();
        m_manager->markAttendance(courseId, studentId, date, present);
    }

    QMessageBox::information(this, "Saved", "Attendance has been saved successfully!");
}

void TeacherAttendanceDialog::onExportAttendance()
{
    int courseId = m_comboCourse->currentData().toInt();
    if (courseId <= 0)
        return;

    QString courseName = m_comboCourse->currentText();
    QString defaultName = QString("attendance_%1.csv").arg(courseName.replace(" ", "_"));

    QString path = QFileDialog::getSaveFileName(this, "Export Attendance", defaultName, "CSV Files (*.csv)");
    if (path.isEmpty())
        return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, "Error", "Could not open file for writing.");
        return;
    }

    QTextStream out(&file);

    // Header row
    out << "ID,Student Name";
    for (const auto &dateStr : m_allDates)
    {
        out << "," << dateStr;
    }
    out << ",Total Present,Total Absent,Absent %\n";

    // Data rows
    for (const auto &stu : m_students)
    {
        out << stu.studentId << "," << stu.name;

        int presentCount = 0;
        for (const auto &dateStr : m_allDates)
        {
            auto key = QPair<int, QString>(stu.studentId, dateStr);
            bool present = m_attendanceCache.value(key, false);
            out << "," << (present ? "P" : "A");
            if (present)
                presentCount++;
        }

        int absentCount = m_allDates.size() - presentCount;
        double absentPct = (m_allDates.size() > 0)
                               ? (double)absentCount / m_allDates.size() * 100.0
                               : 0.0;

        out << "," << presentCount << "," << absentCount
            << "," << QString::number(absentPct, 'f', 1) << "%\n";
    }

    file.close();
    QMessageBox::information(this, "Exported", "Attendance exported to:\n" + path);
}

void TeacherAttendanceDialog::updateSummaryStats()
{
    if (m_students.isEmpty() || m_allDates.isEmpty())
    {
        m_lblStats->setText("No data available.");
        return;
    }

    int totalPresent = 0;
    int totalEntries = m_students.size() * m_allDates.size();

    for (const auto &stu : m_students)
    {
        for (const auto &dateStr : m_allDates)
        {
            auto key = QPair<int, QString>(stu.studentId, dateStr);
            if (m_attendanceCache.value(key, false))
                totalPresent++;
        }
    }

    double overallPct = (totalEntries > 0) ? (double)totalPresent / totalEntries * 100.0 : 0.0;

    // Count students below 75%
    int lowAttendanceCount = 0;
    for (const auto &stu : m_students)
    {
        int present = 0;
        for (const auto &dateStr : m_allDates)
        {
            auto key = QPair<int, QString>(stu.studentId, dateStr);
            if (m_attendanceCache.value(key, false))
                present++;
        }
        double pct = (m_allDates.size() > 0) ? (double)present / m_allDates.size() * 100.0 : 0.0;
        if (pct < 75.0)
            lowAttendanceCount++;
    }

    m_lblStats->setText(
        QString("Total Students: %1 | Classes Held: %2 | Overall Attendance: %3% | Students below 75%%: %4")
            .arg(m_students.size())
            .arg(m_allDates.size())
            .arg(QString::number(overallPct, 'f', 1))
            .arg(lowAttendanceCount));
}
