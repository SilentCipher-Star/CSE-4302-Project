#include "../include/mainwindow.hpp"
#include "../include/utils.hpp"
#include "../include/theme.hpp"
#include "ui_mainwindow.h"
#include "../include/csvhandler.hpp"
#include <QInputDialog>
#include <QMessageBox>
#include <QApplication>
#include <QCheckBox>
#include <QDate>
#include <algorithm>
#include <cmath>
#include <QStandardItemModel>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QResizeEvent>
#include <QScrollBar>
#include <QStyle>
#include <QTimer>
#include <QVector>

MainWindow::MainWindow(QString role, int uid, QString name, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), userRole(role), userId(uid), userName(name)
{
    activeTimerHabit = nullptr;
    ui->setupUi(this);

    // Logout button in the top right corner of the tab widget
    ui->tabWidget->setCornerWidget(ui->logoutButton, Qt::TopRightCorner);

    // Start in Full Screen
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowState(Qt::WindowMaximized);

    // Initialize UI components
    setupTables();
    setupTimers();
    setupConnections();

    ui->label_welcome->setText("Welcome, " + name);

    // Add shadow effect to profile box
    QGraphicsDropShadowEffect *profileShadow = new QGraphicsDropShadowEffect(ui->groupBox_profile);
    profileShadow->setBlurRadius(20);
    profileShadow->setXOffset(0);
    profileShadow->setYOffset(5);
    profileShadow->setColor(QColor(0, 0, 0, 40));
    ui->groupBox_profile->setGraphicsEffect(profileShadow);

    // Setup Admin View Model with Proxy for filtering
    adminModel = new QStandardItemModel(this);

    adminProxyModel = new QSortFilterProxyModel(this);
    adminProxyModel->setSourceModel(adminModel);
    adminProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    adminProxyModel->setFilterKeyColumn(-1); // Filter all columns
    ui->adminTableView->setModel(adminProxyModel);
    csvDelegate = new CsvDelegate(this);
    ui->adminTableView->setItemDelegate(csvDelegate);

    QStringList tables = {
        "admins", "students", "teachers", "courses", "routine", "routine_adjustments", "grades", "notices"};
    ui->tableComboBox->addItems(tables);

    if (!tables.isEmpty())
        on_tableComboBox_currentTextChanged(tables.first());

    // Adjust UI based on User Role
    if (role == "Student")
    {
        ui->addNoticeButton->setVisible(false);
        ui->tabWidget->setTabVisible(5, false);
        ui->tabWidget->setTabVisible(6, false);
        ui->tabWidget->setTabVisible(7, false);
        ui->tabWidget->setTabVisible(8, false);
        ui->tabWidget->setTabVisible(10, false);
    }
    else if (role == "Teacher")
    {
        ui->tabWidget->setTabVisible(1, false);
        ui->tabWidget->setTabVisible(2, false);
        ui->tabWidget->setTabVisible(3, false);
        ui->tabWidget->setTabVisible(4, false);
        ui->tabWidget->setTabVisible(10, false);
        refreshTeacherRoutine();
        refreshTeacherTools();
    }
    else if (role == "Admin")
    {
        ui->tabWidget->setTabVisible(1, false);
        ui->tabWidget->setTabVisible(2, false);
        ui->tabWidget->setTabVisible(3, false);
        ui->tabWidget->setTabVisible(4, false);
        ui->tabWidget->setTabVisible(5, false);
        ui->tabWidget->setTabVisible(6, false);
        ui->tabWidget->setTabVisible(7, false);
        ui->tabWidget->setTabVisible(8, false);
        ui->tabWidget->setTabVisible(9, false);
        ui->addNoticeButton->setVisible(false);
        ui->label_notices->setVisible(false);
        ui->noticeListWidget->setVisible(false);
    }

    // Load initial data
    try
    {
        refreshDashboard();
        refreshQueries();
        if (role == "Student")
        {
            refreshPlanner();
            refreshHabits();

            int currentDayIndex = QDate::currentDate().dayOfWeek() % 7;
            ui->comboRoutineDay->setCurrentIndex(currentDayIndex);

            refreshAcademics();
        }
    }
    catch (const Acadence::Exception &e)
    {
        QMessageBox::critical(this, "Data Error", QString("Failed to load initial data:\n%1").arg(e.what()));
    }
}

MainWindow::~MainWindow()
{
    qDeleteAll(currentHabitList);
    delete ui;
}

// Configures table headers to be interactive
void MainWindow::setupTables()
{
    // Configure all tables to have interactive resizing
    QList<QTableView *> tables = {
        ui->adminTableView, ui->tableRoutine, ui->tableTeacherRoutine,
        ui->tableAcademics, ui->tableGrading, ui->tableAttendance};

    for (auto table : tables)
    {
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        table->horizontalHeader()->setStretchLastSection(false);
    }
}

// Initializes Focus and Workout timers with signal connections
void MainWindow::setupTimers()
{
    // Focus Timer
    ui->label_timerDisplay->setText("00:00");
    ui->label_timerDisplay->setAlignment(Qt::AlignCenter);
    m_focusTimer = new Timer(this);

    connect(m_focusTimer, &Timer::timeUpdated, [this](QString time, float progress)
            {
        ui->label_timerDisplay->setText(time);
        QString style = QString("QLabel { font-size: %1; border: 2px solid palette(base); border-radius: 12px; color: palette(text); "
                                "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                                "stop:0 palette(base), stop:%2 palette(base), "
                                "stop:%3 transparent, stop:1 transparent); }")
                            .arg(AppFonts::Timer)
                            .arg(progress > 0.001 ? progress - 0.001 : 0)
                            .arg(progress);
        ui->label_timerDisplay->setStyleSheet(style); });

    connect(m_focusTimer, &Timer::finished, [this]()
            { QMessageBox::information(this, "Timer", "Focus session complete!"); });

    // Workout/Habit Timer
    ui->label_workoutTimerDisplay->setText("00:00");
    ui->label_workoutTimerDisplay->setAlignment(Qt::AlignCenter);
    m_workoutTimer = new Timer(this);

    connect(m_workoutTimer, &Timer::timeUpdated, [this](QString time, float progress)
            {
        ui->label_workoutTimerDisplay->setText(time);
        QString style = QString("QLabel { font-size: %1; border: 2px solid palette(base); border-radius: 12px; color: palette(text); "
                                "background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
                                "stop:0 palette(base), stop:%2 palette(base), "
                                "stop:%3 transparent, stop:1 transparent); }")
                            .arg(AppFonts::Timer)
                            .arg(progress > 0.001 ? progress - 0.001 : 0)
                            .arg(progress);
        ui->label_workoutTimerDisplay->setStyleSheet(style); });

    connect(m_workoutTimer, &Timer::finished, [this]()
            {
        QMessageBox::information(this, "Habit", "Session complete!");
        if (activeTimerHabit) {
            if (auto wh = dynamic_cast<WorkoutHabit*>(activeTimerHabit)) {
                bool ok;
                int count = QInputDialog::getInt(this, "Workout Progress", "Session Done! Add Reps/Count:", 0, 0, 1000, 1, &ok);
                if (ok && count > 0) {
                    wh->currentCount += count;
                }
                wh->currentMinutes = wh->targetMinutes;
                wh->markComplete();
                myManager.updateHabit(wh);
            } else if (auto dh = dynamic_cast<DurationHabit*>(activeTimerHabit)) {
                dh->currentMinutes = dh->targetMinutes;
                dh->markComplete();
                myManager.updateHabit(dh);
            }
            refreshHabits();
        } });
}

// Connects dynamic UI elements and signals
void MainWindow::setupConnections()
{
    connect(ui->btnChangePassword, &QPushButton::clicked, this, &MainWindow::onChangePasswordClicked);

    // Study Planner UI Setup

    ui->addTaskButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->btnDeleteTask->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(ui->btnDeleteTask, &QPushButton::clicked, this, &MainWindow::on_btnDeleteTask_clicked);
    connect(ui->taskListWidget, &QListWidget::itemChanged, this, &MainWindow::on_taskItemChanged);
}

// Handles password change dialog and validation
void MainWindow::onChangePasswordClicked()
{
    QDialog dlg(this);
    dlg.setWindowTitle("Change Password");
    dlg.setModal(true);
    QFormLayout *layout = new QFormLayout(&dlg);

    QLineEdit *oldPass = new QLineEdit(&dlg);
    oldPass->setEchoMode(QLineEdit::Password);
    layout->addRow("Old Password:", oldPass);

    QLineEdit *newPass = new QLineEdit(&dlg);
    newPass->setEchoMode(QLineEdit::Password);
    layout->addRow("New Password:", newPass);

    QLineEdit *confirmPass = new QLineEdit(&dlg);
    confirmPass->setEchoMode(QLineEdit::Password);
    layout->addRow("Confirm Password:", confirmPass);

    QDialogButtonBox *btns = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dlg);
    layout->addRow(btns);

    connect(btns, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
    connect(btns, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

    if (dlg.exec() == QDialog::Accepted)
    {
        if (newPass->text() != confirmPass->text())
        {
            QMessageBox::warning(this, "Error", "New passwords do not match.");
            return;
        }

        QString error = Utils::validatePassword(newPass->text());
        if (!error.isEmpty())
        {
            QMessageBox::warning(this, "Validation Error", error);
            return;
        }

        try
        {
            if (myManager.changePassword(userId, userRole, oldPass->text(), newPass->text()))
            {
                QMessageBox::information(this, "Success", "Password changed successfully.");
            }
        }
        catch (const Acadence::Exception &e)
        {
            QMessageBox::critical(this, "Error", e.what());
        }
    }
}

// Reloads dashboard notices and profile information
void MainWindow::refreshDashboard()
{
    ui->noticeListWidget->clear();
    QVector<Notice> notices;
    try
    {
        notices = myManager.getNotices();
    }
    catch (const Acadence::Exception &e)
    {
    }
    for (const auto &n : notices)
    {
        ui->noticeListWidget->addItem("[" + n.getDate() + "] " + n.getAuthor() + ": " + n.getContent());
    }

    QString stats = myManager.getDashboardStats(userId, userRole);
    ui->label_welcome->setToolTip(stats);

    if (userRole == "Student")
    {
        Student *s = myManager.getStudent(userId);
        if (s)
        {
            ui->val_p_name->setText(s->getName());
            ui->lbl_p_id->setVisible(true);
            ui->val_p_id->setVisible(true);
            ui->val_p_id->setText(QString::number(s->getId()));
            ui->val_p_dept->setText(s->getDepartment());
            ui->val_p_sem->setText(QString::number(s->getSemester()));
            ui->val_p_email->setText(s->getEmail());
            delete s;
        }
    }
    else if (userRole == "Teacher")
    {
        Teacher *t = myManager.getTeacher(userId);
        if (t)
        {
            ui->val_p_name->setText(t->getName());
            ui->lbl_p_id->setVisible(false);
            ui->val_p_id->setVisible(false);
            ui->val_p_dept->setText(t->getDepartment());
            ui->lbl_p_sem->setText("Designation:");
            ui->val_p_sem->setText(t->getDesignation());
            ui->val_p_email->setText(t->getEmail());
            delete t;
        }
    }
    else if (userRole == "Admin")
    {
        ui->groupBox_profile->setTitle("Administrator");
        ui->val_p_name->setText("System Admin");
        ui->lbl_p_id->setVisible(false);
        ui->val_p_id->setVisible(false);
        ui->val_p_dept->setText("IT / Admin");
        ui->lbl_p_sem->setText("Role:");
        ui->val_p_sem->setText("Super User");
        ui->val_p_email->setText("admin@school.edu");
    }
}

// Opens dialog to post a new notice
void MainWindow::on_addNoticeButton_clicked()
{
    bool ok;
    QString content = QInputDialog::getText(this, "Post Notice", "Content:", QLineEdit::Normal, "", &ok);
    if (ok && !content.isEmpty())
    {
        try
        {
            myManager.addNotice(content, userName);
        }
        catch (const Acadence::Exception &e)
        {
            QMessageBox::critical(this, "Error", e.what());
        }
        refreshDashboard();
    }
}

// Exits the application with a specific code to trigger re-login
void MainWindow::on_logoutButton_clicked()
{
    QApplication::exit(99);
}

// Reloads the To-Do list
void MainWindow::refreshPlanner()
{
    ui->taskListWidget->blockSignals(true); // Prevent triggering itemChanged during reload
    ui->taskListWidget->clear();
    QVector<Task> tasks = myManager.getTasks(userId);
    for (const auto &t : tasks)
    {
        QListWidgetItem *item = new QListWidgetItem(t.getDescription());
        item->setData(Qt::UserRole, t.getId());
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(t.getIsCompleted() ? Qt::Checked : Qt::Unchecked);

        if (t.getIsCompleted())
        {
            QFont f = item->font();
            f.setStrikeOut(true);
            item->setFont(f);
            item->setForeground(Qt::gray);
        }
        ui->taskListWidget->addItem(item);
    }
    ui->taskListWidget->blockSignals(false);
}

// Adds a new task to the planner
void MainWindow::on_addTaskButton_clicked()
{
    QString desc = ui->taskLineEdit->text();
    if (!desc.isEmpty())
    {
        myManager.addTask(userId, desc);
        ui->taskLineEdit->clear();
        refreshPlanner();
    }
}

// Deletes the selected task
void MainWindow::on_btnDeleteTask_clicked()
{
    QListWidgetItem *item = ui->taskListWidget->currentItem();
    if (item)
    {
        int id = item->data(Qt::UserRole).toInt();
        myManager.deleteTask(id);
        delete ui->taskListWidget->takeItem(ui->taskListWidget->row(item));
    }
}

// Updates task completion status when checkbox is toggled
void MainWindow::on_taskItemChanged(QListWidgetItem *item)
{
    int id = item->data(Qt::UserRole).toInt();
    bool isChecked = (item->checkState() == Qt::Checked);
    myManager.completeTask(id, isChecked);

    // Update visual style for checked boxes
    QFont f = item->font();
    f.setStrikeOut(isChecked);
    item->setFont(f);
    item->setForeground(isChecked ? Qt::gray : QPalette().color(QPalette::Text));
}

// Focus Timer Controls
void MainWindow::on_btnTimerStart_clicked()
{
    m_focusTimer->start(ui->spinTimerMinutes->value());
}

void MainWindow::on_btnTimerPause_clicked()
{
    m_focusTimer->pause();
}

void MainWindow::on_btnTimerStop_clicked()
{
    m_focusTimer->stop();
}

// Reloads habits and prayer status
void MainWindow::refreshHabits()
{
    DailyPrayerStatus prayers = myManager.getDailyPrayers(userId, QDate::currentDate().toString(Qt::ISODate));
    ui->chkFajr->setChecked(prayers.getFajr());
    ui->chkDhuhr->setChecked(prayers.getDhuhr());
    ui->chkAsr->setChecked(prayers.getAsr());
    ui->chkMaghrib->setChecked(prayers.getMaghrib());
    ui->chkIsha->setChecked(prayers.getIsha());

    qDeleteAll(currentHabitList);
    currentHabitList = myManager.getHabits(userId);
    ui->habitListWidget->clear();

    for (auto h : currentHabitList)
    {
        QString label = QString("[%1] %2 | %3 | Streak: %4 %5")
                            .arg(h->getTypeString())
                            .arg(h->name)
                            .arg(h->getProgressString())
                            .arg(h->streak)
                            .arg(h->isCompleted ? "[DONE]" : "");

        QListWidgetItem *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, h->id);
        if (h->isCompleted)
            item->setForeground(Qt::green);
        ui->habitListWidget->addItem(item);
    }
}

// Opens dialog to create a new habit
void MainWindow::on_btnAddHabit_clicked()
{
    QStringList types = {"Duration (Timer)", "Count (Counter)", "Workout (Both)"};
    bool ok;
    QString typeStr = QInputDialog::getItem(this, "Create Habit", "Type:", types, 0, false, &ok);
    if (!ok)
        return;

    QString name = QInputDialog::getText(this, "Create Habit", "Name:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty())
        return;

    QStringList freqs = {"Daily", "Weekly"};
    QString freqStr = QInputDialog::getItem(this, "Create Habit", "Frequency:", freqs, 0, false, &ok);
    Frequency f = (freqStr == "Daily") ? Frequency::DAILY : Frequency::WEEKLY;

    if (typeStr.startsWith("Duration"))
    {
        int target = QInputDialog::getInt(this, "Create Habit", "Target Minutes:", 30, 1, 1440, 1, &ok);
        if (!ok)
            return;
        DurationHabit *h = new DurationHabit(0, userId, name, f, target);
        myManager.addHabit(h);
        delete h;
    }
    else if (typeStr.startsWith("Count"))
    {
        int target = QInputDialog::getInt(this, "Create Habit", "Target Count:", 5, 1, 1000, 1, &ok);
        if (!ok)
            return;
        QString unit = QInputDialog::getText(this, "Create Habit", "Unit:", QLineEdit::Normal, "", &ok);
        CountHabit *h = new CountHabit(0, userId, name, f, target, unit);
        myManager.addHabit(h);
        delete h;
    }
    else if (typeStr.startsWith("Workout"))
    {
        int targetMin = QInputDialog::getInt(this, "Create Habit", "Target Minutes:", 30, 1, 1440, 1, &ok);
        if (!ok)
            return;

        int targetCnt = QInputDialog::getInt(this, "Create Habit", "Target Count/Reps:", 10, 1, 1000, 1, &ok);
        if (!ok)
            return;

        QString unit = QInputDialog::getText(this, "Create Habit", "Unit (e.g., pushups):", QLineEdit::Normal, "", &ok);
        if (!ok)
            return;

        WorkoutHabit *h = new WorkoutHabit(0, userId, name, f, targetMin, targetCnt, unit);
        myManager.addHabit(h);
        delete h;
    }
    refreshHabits();
}

// Increments habit progress or sets up the workout timer
void MainWindow::on_btnPerformHabit_clicked()
{
    int row = ui->habitListWidget->currentRow();
    if (row < 0 || row >= currentHabitList.size())
        return;

    Habit *h = currentHabitList[row];
    bool changed = false;

    if (auto wh = dynamic_cast<WorkoutHabit *>(h))
    {
        activeTimerHabit = wh;
        ui->groupBox_workoutTimer->setTitle("Workout: " + wh->name);
        ui->spinWorkoutMinutes->setValue(wh->targetMinutes - wh->currentMinutes);
        if (ui->spinWorkoutMinutes->value() <= 0)
            ui->spinWorkoutMinutes->setValue(wh->targetMinutes);

        QMessageBox::information(this, "Workout", "Timer set for '" + wh->name + "'.\nDon't forget to log your reps manually if needed!");
    }
    else if (auto dh = dynamic_cast<DurationHabit *>(h))
    {
        activeTimerHabit = dh;
        ui->groupBox_workoutTimer->setTitle("Stopwatch: " + dh->name);
        ui->spinWorkoutMinutes->setEnabled(false); // Disable for stopwatch mode
        ui->label_workoutTimerDisplay->setText("00:00.00");

        QMessageBox::information(this, "Stopwatch Ready", "Stopwatch ready for '" + dh->name + "'.\nClick Start to begin tracking.");
    }
    else if (auto ch = dynamic_cast<CountHabit *>(h))
    {
        bool ok;
        int count = QInputDialog::getInt(this, "Perform Habit", "Add Count:", 1, 1, 100, 1, &ok);
        if (ok)
        {
            ch->currentCount += count;
            if (ch->currentCount >= ch->targetCount)
                ch->markComplete();
            changed = true;
        }
    }

    if (changed)
    {
        myManager.updateHabit(h);
        refreshHabits();
    }
}

// Deletes the selected habit
void MainWindow::on_btnDeleteHabit_clicked()
{
    int row = ui->habitListWidget->currentRow();
    if (row < 0 || row >= currentHabitList.size())
        return;

    int id = currentHabitList[row]->id;
    myManager.deleteHabit(id);
    refreshHabits();
}

// Workout Timer Controls
void MainWindow::on_btnWorkoutStart_clicked()
{
    if (auto wh = dynamic_cast<WorkoutHabit *>(activeTimerHabit))
    {
        ui->spinWorkoutMinutes->setEnabled(true);
        m_workoutTimer->start(ui->spinWorkoutMinutes->value());
    }
    else if (auto dh = dynamic_cast<DurationHabit *>(activeTimerHabit))
    {
        // Pure DurationHabit uses stopwatch
        ui->spinWorkoutMinutes->setEnabled(false);
        m_workoutTimer->startStopwatch(dh->targetMinutes);
    }
}

void MainWindow::on_btnWorkoutPause_clicked()
{
    m_workoutTimer->pause();
}

void MainWindow::on_btnWorkoutStop_clicked()
{
    if (activeTimerHabit)
    {
        double elapsed = m_workoutTimer->getElapsedMinutes();
        if (elapsed > 0)
        {
            activeTimerHabit->currentMinutes += elapsed;
            if (activeTimerHabit->currentMinutes >= activeTimerHabit->targetMinutes)
                activeTimerHabit->markComplete();

            myManager.updateHabit(activeTimerHabit);
            refreshHabits();
        }
    }
    m_workoutTimer->stop();
}

// Prayer Tracker Toggles
void MainWindow::on_chkFajr_toggled(bool checked)
{
    myManager.updateDailyPrayer(userId, QDate::currentDate().toString(Qt::ISODate), "fajr", checked);
}
void MainWindow::on_chkDhuhr_toggled(bool checked)
{
    myManager.updateDailyPrayer(userId, QDate::currentDate().toString(Qt::ISODate), "dhuhr", checked);
}
void MainWindow::on_chkAsr_toggled(bool checked)
{
    myManager.updateDailyPrayer(userId, QDate::currentDate().toString(Qt::ISODate), "asr", checked);
}
void MainWindow::on_chkMaghrib_toggled(bool checked)
{
    myManager.updateDailyPrayer(userId, QDate::currentDate().toString(Qt::ISODate), "maghrib", checked);
}
void MainWindow::on_chkIsha_toggled(bool checked)
{
    myManager.updateDailyPrayer(userId, QDate::currentDate().toString(Qt::ISODate), "isha", checked);
}

// Calculates the date for a specific day of the current week
QDate MainWindow::getDateForDay(QString dayName)
{
    QDate today = QDate::currentDate();
    int currentDayOfWeek = today.dayOfWeek(); // 1=Mon, 7=Sun

    QStringList days = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    int targetDayOfWeek = days.indexOf(dayName) + 1;

    int diff = targetDayOfWeek - currentDayOfWeek;
    return today.addDays(diff);
}

// Reloads the student routine table
void MainWindow::refreshRoutine()
{
    ui->tableRoutine->setRowCount(0);
    QString day = ui->comboRoutineDay->currentText();

    int semester = -1;
    if (userRole == "Student")
    {
        Student *s = myManager.getStudent(userId);
        if (s)
        {
            semester = s->getSemester();
            delete s;
        }
    }

    QVector<RoutineSession> items = myManager.getEffectiveRoutine(getDateForDay(day), semester);

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

void MainWindow::on_comboRoutineDay_currentIndexChanged(int index)
{
    refreshRoutine();
}

// Reloads the teacher routine table, filtering by assigned courses
void MainWindow::refreshTeacherRoutine()
{
    ui->tableTeacherRoutine->setRowCount(0);
    QString day = ui->comboRoutineDayInput->currentText();
    QVector<RoutineSession> items = myManager.getEffectiveRoutine(getDateForDay(day));

    if (userRole == "Teacher")
    {
        QVector<Course *> courses = myManager.getTeacherCourses(userId);
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

        // Store hidden data for logic
        ui->tableTeacherRoutine->item(row, 0)->setData(Qt::UserRole, i.getStartTime());
        ui->tableTeacherRoutine->item(row, 1)->setData(Qt::UserRole, i.getCourseCode());
    }
    Utils::adjustColumnWidths(ui->tableTeacherRoutine);
}

void MainWindow::on_comboRoutineDayInput_currentIndexChanged(int index)
{
    refreshTeacherRoutine();
}

// Cancels a selected class for the teacher
void MainWindow::on_btnCancelClass_clicked()
{
    int row = ui->tableTeacherRoutine->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(this, "Selection", "Please select a class to cancel.");
        return;
    }

    QString day = ui->comboRoutineDayInput->currentText();
    QDate date = getDateForDay(day);
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
    reply = QMessageBox::question(this, "Confirm Cancellation",
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

        myManager.addRoutineAdjustment(adj);
        refreshTeacherRoutine();
        QMessageBox::information(this, "Success", "Class cancelled.");
    }
}

// Reschedules or exchanges a selected class
void MainWindow::on_btnRescheduleClass_clicked()
{
    int row = ui->tableTeacherRoutine->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(this, "Selection", "Please select a class to exchange/reschedule.");
        return;
    }

    QString currentDayStr = ui->comboRoutineDayInput->currentText();
    QDate originDate = getDateForDay(currentDayStr);
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

    // Find available slots or exchangeable slots for next 2 weeks
    QStringList options;
    QVector<RoutineAdjustment> potentialAdjustments;
    QVector<RoutineAdjustment> secondaryAdjustments;

    QDate d = QDate::currentDate();
    for (int i = 0; i < 14; ++i)
    {
        QDate targetDate = d.addDays(i);
        if (targetDate.dayOfWeek() > 5)
            continue; // Skip weekends

        QVector<RoutineSession> daily = myManager.getEffectiveRoutine(targetDate, semester);

        // Check standard slots: 09:00, 10:00, 11:00, 12:00
        QList<int> serials = {1, 2, 3, 4, 5};
        for (int s : serials)
        {
            QString slotTime;
            if (s == 1)
                slotTime = "09:00";
            else if (s == 2)
                slotTime = "10:00";
            else if (s == 3)
                slotTime = "11:00";
            else if (s == 4)
                slotTime = "12:00";
            else if (s == 5)
                slotTime = "14:00";

            if (targetDate == originDate && s == originSerial)
                continue;

            bool occupied = false;
            RoutineSession occupiedSession("", "", "", "", "", "", "", 0);

            for (const auto &sess : daily)
            {
                if (sess.getStartTime() == slotTime)
                {
                    occupied = true;
                    occupiedSession = sess;
                    break;
                }
            }

            RoutineAdjustment adj;
            adj.originalDate = originDate.toString(Qt::ISODate);
            adj.originalSerial = originSerial;
            adj.type = "RESCHEDULE";
            adj.newDate = targetDate.toString(Qt::ISODate);
            adj.newSerial = s;
            adj.courseCode = originCode;
            adj.courseName = ui->tableTeacherRoutine->item(row, 1)->text() + "(Rescheduled)";
            adj.room = originRoom;
            adj.instructor = userName;
            adj.semester = semester;

            RoutineAdjustment adj2;

            if (!occupied)
            {
                options << targetDate.toString("ddd MMM dd") + " at " + slotTime + " (Empty Slot)";
                potentialAdjustments.append(adj);
                secondaryAdjustments.append(adj2);
            }
            else
            {
                // Exchange option
                options << targetDate.toString("ddd MMM dd") + " at " + slotTime + " (Exchange with " + occupiedSession.getCourseCode() + ")";

                adj2.originalDate = targetDate.toString(Qt::ISODate);
                adj2.originalSerial = s;
                adj2.type = "RESCHEDULE";
                adj2.newDate = originDate.toString(Qt::ISODate);
                adj2.newSerial = originSerial;
                adj2.courseCode = occupiedSession.getCourseCode();
                adj2.courseName = occupiedSession.getCourseName();
                adj2.room = occupiedSession.getRoom();
                adj2.instructor = occupiedSession.getInstructor();
                adj2.semester = occupiedSession.getSemester();

                potentialAdjustments.append(adj);
                secondaryAdjustments.append(adj2);
            }
        }
    }

    bool ok;
    QString choice = QInputDialog::getItem(this, "Select New Slot", "Available Options (Next 2 Weeks):", options, 0, false, &ok);
    if (ok && !choice.isEmpty())
    {
        int idx = options.indexOf(choice);
        if (idx >= 0 && idx < potentialAdjustments.size())
        {
            // Apply the adjustment
            myManager.addRoutineAdjustment(potentialAdjustments[idx]);

            if (!secondaryAdjustments[idx].courseCode.isEmpty())
            {
                myManager.addRoutineAdjustment(secondaryAdjustments[idx]);
            }

            refreshTeacherRoutine();
            QMessageBox::information(this, "Success", "Class rescheduled/exchanged successfully.");
        }
    }
}

// Reloads student academic data (attendance, assessments)
void MainWindow::refreshAcademics()
{
    ui->listAssessments->clear();
    QVector<Assessment> assessments = myManager.getAssessments();
    for (const auto &a : assessments)
    {
        ui->listAssessments->addItem(a.getDate() + " - " + a.getCourseName() + ": " + a.getTitle() + " (" + a.getType() + ")");
    }

    ui->tableAcademics->setRowCount(0);
    QVector<AttendanceRecord> att = myManager.getStudentAttendance(userId);
    for (int i = 0; i < att.size(); ++i)
    {
        ui->tableAcademics->insertRow(i);
        ui->tableAcademics->setItem(i, 0, new QTableWidgetItem(att[i].getCourseName()));

        double pct = (att[i].getTotalClasses() > 0) ? (double)att[i].getAttendedClasses() / att[i].getTotalClasses() * 100.0 : 0.0;
        QTableWidgetItem *pctItem = new QTableWidgetItem(QString::number(pct, 'f', 1) + "%");
        if (pct < 85.0)
            pctItem->setForeground(Qt::red);
        ui->tableAcademics->setItem(i, 1, pctItem);

        QString scoreStr = QString::number(att[i].getTotalMarksObtained()) + " / " + QString::number(att[i].getTotalMaxMarks());
        ui->tableAcademics->setItem(i, 2, new QTableWidgetItem(scoreStr));

        QString status = (pct < 85.0) ? "Low Attendance" : "Good";
        ui->tableAcademics->setItem(i, 3, new QTableWidgetItem(status));
    }
    Utils::adjustColumnWidths(ui->tableAcademics);
}

// Refreshes dropdowns and tables for teacher tools
void MainWindow::refreshTeacherTools()
{
    ui->comboTeacherCourse->clear();
    ui->comboAttendanceCourse->clear();
    QVector<Course *> courses = myManager.getTeacherCourses(userId);
    for (auto c : courses)
    {
        ui->comboTeacherCourse->addItem(c->getName(), c->getId());
        ui->comboAttendanceCourse->addItem(c->getCode() + " - " + c->getName(), c->getId());
    }
    qDeleteAll(courses);

    ui->comboTeacherAssessment->clear();
    QVector<Assessment> assessments = myManager.getAssessments();
    for (const auto &a : assessments)
    {
        ui->comboTeacherAssessment->addItem(a.getCourseName() + " - " + a.getTitle(), a.getId());
    }

    refreshTeacherGrades();
    refreshTeacherAttendance();
}

// Creates a new assessment
void MainWindow::on_btnCreateAssessment_clicked()
{
    int courseId = ui->comboTeacherCourse->currentData().toInt();
    QString title = ui->editAssessmentTitle->text();
    if (title.isEmpty())
        return;

    myManager.addAssessment(courseId, title, ui->comboAssessmentType->currentText(),
                            QDate::currentDate().toString("yyyy-MM-dd"), ui->spinMaxMarks->value());
    ui->editAssessmentTitle->clear();
    refreshTeacherTools();
    QMessageBox::information(this, "Success", "Assessment Created");
}

void MainWindow::on_comboTeacherAssessment_currentIndexChanged(int index)
{
    refreshTeacherGrades();
}

// Reloads the grading table for the selected assessment
void MainWindow::refreshTeacherGrades()
{
    ui->tableGrading->setRowCount(0);

    int assessmentId = ui->comboTeacherAssessment->currentData().toInt();

    QVector<Assessment> allAssessments = myManager.getAssessments();
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

    Course *c = myManager.getCourse(courseId);

    QVector<Student *> students;
    if (c)
    {
        students = myManager.getStudentsBySemester(c->getSemester());
        delete c;
    }

    for (int i = 0; i < students.size(); ++i)
    {
        ui->tableGrading->insertRow(i);
        ui->tableGrading->setItem(i, 0, new QTableWidgetItem(QString::number(students[i]->getId())));
        ui->tableGrading->setItem(i, 1, new QTableWidgetItem(students[i]->getName()));

        double currentGrade = myManager.getGrade(students[i]->getId(), assessmentId);
        QString gradeStr = (currentGrade >= 0) ? QString::number(currentGrade) : "";
        ui->tableGrading->setItem(i, 2, new QTableWidgetItem(gradeStr));
    }
    qDeleteAll(students);
    Utils::adjustColumnWidths(ui->tableGrading);
}

// Saves entered grades to the database
void MainWindow::on_btnSaveGrades_clicked()
{
    int assessmentId = ui->comboTeacherAssessment->currentData().toInt();
    int rows = ui->tableGrading->rowCount();
    for (int i = 0; i < rows; ++i)
    {
        int sid = ui->tableGrading->item(i, 0)->text().toInt();
        QString text = ui->tableGrading->item(i, 2)->text();
        if (text.isEmpty())
            continue;

        double marks = text.toDouble();
        myManager.addGrade(sid, assessmentId, marks);
    }
    QMessageBox::information(this, "Success", "Grades Saved");
    refreshTeacherGrades();
}

// Reloads the attendance sheet for the selected course
void MainWindow::refreshTeacherAttendance()
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
    Course *c = myManager.getCourse(courseId);
    if (!c)
        return;

    QVector<Student *> students = myManager.getStudentsBySemester(c->getSemester());
    QVector<QString> dates = myManager.getCourseDates(courseId);

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
            bool present = myManager.isPresent(courseId, sid, dates[j]);
            if (present)
                presentCount++;

            QTableWidgetItem *checkItem = new QTableWidgetItem();
            checkItem->setCheckState(present ? Qt::Checked : Qt::Unchecked);
            ui->tableAttendance->setItem(i, 4 + j, checkItem);
        }

        double pct = dates.isEmpty() ? 0.0 : (double)presentCount / dates.size() * 100.0;
        ui->tableAttendance->setItem(i, 2, new QTableWidgetItem(QString::number(pct, 'f', 1) + "%"));
        ui->tableAttendance->setItem(i, 3, new QTableWidgetItem(QString::number(presentCount) + "/" + QString::number(dates.size())));
    }

    qDeleteAll(students);
    delete c;
    Utils::adjustColumnWidths(ui->tableAttendance);
}

void MainWindow::on_comboAttendanceCourse_currentIndexChanged(int index)
{
    refreshTeacherAttendance();
}

// Saves marked attendance to the database
void MainWindow::on_btnSaveAttendance_clicked()
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
            myManager.markAttendance(courseId, sid, date, present);
        }
    }
    QMessageBox::information(this, "Success", "Attendance Saved");
    refreshTeacherAttendance();
}

// Reloads Q&A threads
void MainWindow::refreshQueries()
{
    ui->listQueries->clear();
    QVector<Query> queries = myManager.getQueries(userId, userRole);

    for (const auto &q : queries)
    {
        QString label;
        if (userRole == "Student")
        {
            label = QString("To: %1 [%2]\nQ: %3\nA: %4").arg(q.getTeacherName(), q.getTimestamp(), q.getQuestion(), q.getAnswer().isEmpty() ? "(Waiting...)" : q.getAnswer());
        }
        else
        {
            label = QString("From: %1 [%2]\nQ: %3\nA: %4").arg(q.getStudentName(), q.getTimestamp(), q.getQuestion(), q.getAnswer().isEmpty() ? "(Select to Reply)" : q.getAnswer());
        }

        QListWidgetItem *item = new QListWidgetItem(label);
        item->setData(Qt::UserRole, q.getId());
        if (q.getAnswer().isEmpty())
            item->setForeground(Qt::red);
        ui->listQueries->addItem(item);
    }
}

// Handles asking a question (Student) or replying (Teacher)
void MainWindow::on_btnQueryAction_clicked()
{
    QString text = ui->editQueryInput->text();
    if (text.isEmpty())
        return;

    if (userRole == "Student")
    {
        QVector<QPair<int, QString>> teachers = myManager.getTeacherList();
        if (teachers.isEmpty())
        {
            QMessageBox::warning(this, "Error", "No teachers found to ask.");
            return;
        }

        QStringList teacherNames;
        for (const auto &t : teachers)
            teacherNames << (t.second + " (ID: " + QString::number(t.first) + ")");

        bool ok;
        QString selected = QInputDialog::getItem(this, "Ask Question", "Select Teacher:", teacherNames, 0, false, &ok);
        if (ok && !selected.isEmpty())
        {
            int teacherId = teachers[teacherNames.indexOf(selected)].first;
            myManager.addQuery(userId, teacherId, text);
            ui->editQueryInput->clear();
            refreshQueries();
        }
    }
    else
    {
        QListWidgetItem *item = ui->listQueries->currentItem();
        if (!item)
        {
            QMessageBox::warning(this, "Selection", "Select a query to reply to.");
            return;
        }
        int qid = item->data(Qt::UserRole).toInt();
        myManager.answerQuery(qid, text);
        ui->editQueryInput->clear();
        refreshQueries();
    }
}

// Helper to write table model data to CSV
void saveTableData(QStandardItemModel *model, const QString &tableName)
{
    QVector<QStringList> data;
    for (int i = 0; i < model->rowCount(); ++i)
    {
        QStringList rowData;
        for (int j = 0; j < model->columnCount(); ++j)
        {
            QStandardItem *item = model->item(i, j);
            rowData << (item ? item->text() : "");
        }
        data.append(rowData);
    }
    CsvHandler::writeCsv(tableName + ".csv", data);
}

// Loads data into the Admin table when selection changes
void MainWindow::on_tableComboBox_currentTextChanged(const QString &tableName)
{
    adminModel->clear();

    disconnect(adminModel, nullptr, this, nullptr);
    csvDelegate->currentTable = tableName;

    QVector<QStringList> data;
    try
    {
        data = CsvHandler::readCsv(tableName + ".csv");
    }
    catch (const Acadence::Exception &e)
    {
        QMessageBox::warning(this, "Load Error", e.what());
    }
    for (const auto &row : data)
    {
        QList<QStandardItem *> items;
        for (const QString &field : row)
            items.append(new QStandardItem(field));
        adminModel->appendRow(items);
    }

    QStringList headers;
    if (tableName == "admins")
        headers << "ID" << "Username" << "Password" << "Name" << "Email";
    else if (tableName == "students")
        headers << "ID" << "Name" << "Email" << "Username" << "Password" << "Dept" << "Batch" << "Sem" << "Admission Date" << "CGPA";
    else if (tableName == "teachers")
        headers << "ID" << "Name" << "Email" << "Username" << "Password" << "Dept" << "Designation" << "Salary";
    else if (tableName == "courses")
        headers << "ID" << "Code" << "Name" << "Teacher ID" << "Semester" << "Credits";
    else if (tableName == "routine")
        headers << "Day" << "Serial" << "Code" << "Name" << "Room" << "Instructor" << "Semester";
    else if (tableName == "routine_adjustments")
        headers << "Orig Date" << "Orig Serial" << "Type" << "New Date" << "New Serial" << "Code" << "Name" << "Room" << "Instructor" << "Semester";
    else if (tableName == "grades")
        headers << "Student ID" << "Assessment ID" << "Marks";
    else if (tableName == "notices")
        headers << "Date" << "Author" << "Content";
    adminModel->setHorizontalHeaderLabels(headers);

    connect(adminModel, &QStandardItemModel::itemChanged, this, [this, tableName]()
            { saveTableData(adminModel, tableName); });

    QTimer::singleShot(10, this, [this]()
                       { Utils::adjustColumnWidths(ui->adminTableView); });
}

// Adds a new row to the Admin table
void MainWindow::on_btnAddRow_clicked()
{
    int row = adminModel->rowCount();
    adminModel->insertRow(row);

    QString currentTable = ui->tableComboBox->currentText();
    QString nextIdStr = "";

    if (currentTable == "admins" || currentTable == "students" ||
        currentTable == "teachers" || currentTable == "courses")
    {
        int maxId = 0;
        for (int i = 0; i < row; ++i)
        {
            QStandardItem *item = adminModel->item(i, 0);
            if (item && item->text().toInt() > maxId)
                maxId = item->text().toInt();
        }
        nextIdStr = QString::number(maxId + 1);
    }

    for (int j = 0; j < adminModel->columnCount(); ++j)
    {
        if (j == 0 && !nextIdStr.isEmpty())
            adminModel->setItem(row, j, new QStandardItem(nextIdStr));
        else
            adminModel->setItem(row, j, new QStandardItem(""));
    }

    saveTableData(adminModel, currentTable);
}

// Deletes selected rows from the Admin table
void MainWindow::on_btnDeleteRow_clicked()
{
    QModelIndexList selected = ui->adminTableView->selectionModel()->selectedRows();
    if (selected.isEmpty())
    {
        QMessageBox::warning(this, "Selection", "Please select a row to delete.");
        return;
    }

    QList<int> sourceRows;
    for (const auto &index : selected)
    {
        QModelIndex sourceIndex = adminProxyModel->mapToSource(index);
        if (sourceIndex.isValid())
            sourceRows.append(sourceIndex.row());
    }

    std::sort(sourceRows.begin(), sourceRows.end());
    for (int i = sourceRows.size() - 1; i >= 0; --i)
    {
        adminModel->removeRow(sourceRows[i]);
    }

    saveTableData(adminModel, ui->tableComboBox->currentText());
}

// Filters the Admin table
void MainWindow::on_searchLineEdit_textChanged(const QString &arg1)
{
    adminProxyModel->setFilterFixedString(arg1);
}

CsvDelegate::CsvDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

// Creates appropriate editors (SpinBox, DateEdit, etc.) based on column type
QWidget *CsvDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int col = index.column();

    if (currentTable == "admins")
    {
        if (col == 0)
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setRange(1, 999999);
            return sb;
        }
    }
    else if (currentTable == "students")
    {
        if (col == 0)
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setRange(1, 999999);
            return sb;
        }
        if (col == 7)
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setRange(1, 8);
            return sb;
        }
        if (col == 8)
        {
            QDateEdit *de = new QDateEdit(parent);
            de->setDisplayFormat("yyyy-MM-dd");
            de->setCalendarPopup(true);
            return de;
        }
        if (col == 9)
        {
            QDoubleSpinBox *dsb = new QDoubleSpinBox(parent);
            dsb->setRange(0.0, 4.0);
            dsb->setSingleStep(0.01);
            return dsb;
        }
        if (col == 5)
        {
            QComboBox *cb = new QComboBox(parent);
            cb->addItems({"CSE", "EEE", "MCE", "CEE", "BTM", "TVE", "SWE"});
            return cb;
        }
    }
    else if (currentTable == "teachers")
    {
        if (col == 0)
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setRange(1, 999999);
            return sb;
        }
        if (col == 7)
        {
            QDoubleSpinBox *dsb = new QDoubleSpinBox(parent);
            dsb->setRange(0.0, 1000000.0);
            return dsb;
        }
        if (col == 5)
        {
            QComboBox *cb = new QComboBox(parent);
            cb->addItems({"CSE", "EEE", "MCE", "CEE", "BTM", "TVE", "SWE"});
            return cb;
        }
    }
    else if (currentTable == "courses")
    {
        if (col == 0 || col == 3 || col == 5)
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setRange(1, 999999);
            return sb;
        }
        if (col == 4)
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setRange(1, 8);
            return sb;
        }
    }
    else if (currentTable == "routine")
    {
        if (col == 1)
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setRange(1, 5);
            return sb;
        }
        if (col == 6)
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setRange(1, 8);
            return sb;
        }
    }
    else if (currentTable == "routine_adjustments")
    {
        if (col == 0 || col == 3)
        {
            QDateEdit *de = new QDateEdit(parent);
            de->setDisplayFormat("yyyy-MM-dd");
            de->setCalendarPopup(true);
            return de;
        }
        if (col == 1 || col == 4)
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setRange(1, 5);
            return sb;
        }
        if (col == 2)
        {
            QComboBox *cb = new QComboBox(parent);
            cb->addItems({"CANCEL", "RESCHEDULE"});
            return cb;
        }
        if (col == 9)
        {
            QSpinBox *sb = new QSpinBox(parent);
            sb->setRange(1, 8);
            return sb;
        }
    }
    else if (currentTable == "notices")
    {
        if (col == 0)
        {
            QDateEdit *de = new QDateEdit(parent);
            de->setDisplayFormat("yyyy-MM-dd");
            de->setCalendarPopup(true);
            return de;
        }
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}

// Populates the editor with data from the model
void CsvDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QString val = index.model()->data(index, Qt::EditRole).toString();
    if (QSpinBox *sb = qobject_cast<QSpinBox *>(editor))
        sb->setValue(val.toInt());
    else if (QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox *>(editor))
        dsb->setValue(val.toDouble());
    else if (QComboBox *cb = qobject_cast<QComboBox *>(editor))
        cb->setCurrentText(val);
    else if (QDateEdit *de = qobject_cast<QDateEdit *>(editor))
        de->setDate(QDate::fromString(val, "yyyy-MM-dd"));
    else
        QStyledItemDelegate::setEditorData(editor, index);
}

// Saves data from the editor to the model, with validation
void CsvDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    int col = index.column();
    QString newVal;

    if (QSpinBox *sb = qobject_cast<QSpinBox *>(editor))
        newVal = QString::number(sb->value());
    else if (QDoubleSpinBox *dsb = qobject_cast<QDoubleSpinBox *>(editor))
        newVal = QString::number(dsb->value());
    else if (QComboBox *cb = qobject_cast<QComboBox *>(editor))
        newVal = cb->currentText();
    else if (QDateEdit *de = qobject_cast<QDateEdit *>(editor))
        newVal = de->date().toString("yyyy-MM-dd");
    else if (QLineEdit *le = qobject_cast<QLineEdit *>(editor))
        newVal = le->text();
    else
    {
        QStyledItemDelegate::setModelData(editor, model, index);
        return;
    }

    // Validate Unique ID
    if (col == 0 && (currentTable == "admins" || currentTable == "students" || currentTable == "teachers" || currentTable == "courses"))
    {
        for (int i = 0; i < model->rowCount(); ++i)
        {
            if (i != index.row() && model->index(i, 0).data(Qt::EditRole).toString() == newVal)
            {
                QMessageBox::warning(editor->parentWidget(), "Validation Error", "ID must be unique.");
                return;
            }
        }
    }

    bool isUsernameCol = (currentTable == "admins" && col == 1) ||
                         ((currentTable == "students" || currentTable == "teachers") && col == 3);

    // Validate Username format and uniqueness
    if (isUsernameCol)
    {
        QString error = Utils::validateUsername(newVal);
        if (!error.isEmpty())
        {
            QMessageBox::warning(editor->parentWidget(), "Validation Error", error);
            return;
        }

        QStringList userTables = {"admins", "students", "teachers"};
        for (const QString &table : userTables)
        {
            int uCol = (table == "admins") ? 1 : 3;
            if (table == currentTable)
            {
                for (int i = 0; i < model->rowCount(); ++i)
                {
                    if (i != index.row() && model->index(i, uCol).data(Qt::EditRole).toString() == newVal)
                    {
                        QMessageBox::warning(editor->parentWidget(), "Validation Error", "Username '" + newVal + "' already taken.");
                        return;
                    }
                }
            }
            else
            {
                QVector<QStringList> data;
                try
                {
                    data = CsvHandler::readCsv(table + ".csv");
                }
                catch (const std::exception &)
                {
                    continue;
                }

                for (const auto &row : data)
                {
                    if (row.size() > uCol && row[uCol] == newVal)
                    {
                        QMessageBox::warning(editor->parentWidget(), "Validation Error", "Username '" + newVal + "' already taken in " + table + ".");
                        return;
                    }
                }
            }
        }
    }

    bool isPasswordCol = (currentTable == "admins" && col == 2) ||
                         ((currentTable == "students" || currentTable == "teachers") && col == 4);

    // Validate Password strength
    if (isPasswordCol)
    {
        QString error = Utils::validatePassword(newVal);
        if (!error.isEmpty())
        {
            QMessageBox::warning(editor->parentWidget(), "Validation Error", error);
            return;
        }
    }

    model->setData(index, newVal, Qt::EditRole);
}

// Event handler to resize columns when the window size changes
void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    QTimer::singleShot(0, this, [=]()
                       {
        if (ui->adminTableView->isVisible())
            Utils::adjustColumnWidths(ui->adminTableView);
        if (ui->tableRoutine->isVisible())
            Utils::adjustColumnWidths(ui->tableRoutine);
        if (ui->tableTeacherRoutine->isVisible())
            Utils::adjustColumnWidths(ui->tableTeacherRoutine);
        if (ui->tableAcademics->isVisible())
            Utils::adjustColumnWidths(ui->tableAcademics);
        if (ui->tableGrading->isVisible())
            Utils::adjustColumnWidths(ui->tableGrading);
        if (ui->tableAttendance->isVisible())
            Utils::adjustColumnWidths(ui->tableAttendance); });
}
