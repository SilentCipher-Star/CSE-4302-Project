#include "../include/mainwindow.hpp"
#include "../include/csvdelegate.hpp"
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
#include <QDialogButtonBox>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QResizeEvent>
#include <QStyle>
#include <QTimer>
#include <QVector>
#include <memory>

MainWindow::MainWindow(QString role, int uid, QString name, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), userRole(role), userId(uid), userName(name)
{
    activeTimerHabit = nullptr;
    myManager.addObserver(this);

    ui->setupUi(this);

    // Setup Logout button
    ui->tabWidget->setCornerWidget(ui->logoutButton, Qt::TopRightCorner);

    // Configure window state
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowState(Qt::WindowMaximized);

    // Setup UI
    setupTables();
    setupTimers();
    setupConnections();

    ui->label_welcome->setText("Welcome, " + name);

    // Apply shadow effect
    QGraphicsDropShadowEffect *profileShadow = new QGraphicsDropShadowEffect(ui->groupBox_profile);
    profileShadow->setBlurRadius(20);
    profileShadow->setXOffset(0);
    profileShadow->setYOffset(5);
    profileShadow->setColor(QColor(0, 0, 0, 40));
    ui->groupBox_profile->setGraphicsEffect(profileShadow);

    // Initialize Admin models
    adminModel = new QStandardItemModel(this);

    adminProxyModel = new QSortFilterProxyModel(this);
    adminProxyModel->setSourceModel(adminModel);
    adminProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    adminProxyModel->setFilterKeyColumn(-1); // Filter all columns
    ui->adminTableView->setModel(adminProxyModel);
    csvDelegate = new CsvDelegate(this);
    ui->adminTableView->setItemDelegate(csvDelegate);

    QStringList tables = {
        Constants::Table::Admins, Constants::Table::Students, Constants::Table::Teachers, Constants::Table::Courses, Constants::Table::Routine, Constants::Table::RoutineAdj, "grades", Constants::Table::Notices};
    ui->tableComboBox->addItems(tables);

    if (!tables.isEmpty())
        on_tableComboBox_currentTextChanged(tables.first());

    // Configure UI based on role
    if (role == Constants::Role::Student)
    {
        ui->addNoticeButton->setVisible(false);
        ui->tabWidget->setTabVisible(5, false);
        ui->tabWidget->setTabVisible(6, false);
        ui->tabWidget->setTabVisible(7, false);
        ui->tabWidget->setTabVisible(8, false);
        ui->tabWidget->setTabVisible(10, false);
    }
    else if (role == Constants::Role::Teacher)
    {
        ui->tabWidget->setTabVisible(1, false);
        ui->tabWidget->setTabVisible(2, false);
        ui->tabWidget->setTabVisible(3, false);
        ui->tabWidget->setTabVisible(4, false);
        ui->tabWidget->setTabVisible(10, false);
        refreshTeacherRoutine();
        refreshTeacherTools();
    }
    else if (role == Constants::Role::Admin)
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

    // Load data
    try
    {
        refreshDashboard();
        refreshQueries();
        if (role == Constants::Role::Student)
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
    myManager.removeObserver(this);
    qDeleteAll(currentHabitList);
    delete ui;
}

// Configure table headers
void MainWindow::setupTables()
{
    // Set interactive resizing
    QList<QTableView *> tables = {
        ui->adminTableView, ui->tableRoutine, ui->tableTeacherRoutine,
        ui->tableAcademics, ui->tableGrading, ui->tableAttendance};

    for (auto table : tables)
    {
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        table->horizontalHeader()->setStretchLastSection(false);
    }
}

// Initialize timers
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
                if (wh->currentCount >= wh->targetCount)
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

void MainWindow::onDataChanged(DataType type)
{
    switch (type)
    {
    case DataType::Habits:
        refreshHabits();
        break;
    case DataType::Tasks:
        refreshPlanner();
        break;
    case DataType::Routine:
        if (userRole == Constants::Role::Teacher)
            refreshTeacherRoutine();
        else
            refreshRoutine();
        break;
    case DataType::Notices:
        refreshDashboard();
        break;
    case DataType::Academics:
        refreshAcademics();
        if (userRole == Constants::Role::Teacher)
            refreshTeacherTools();
        break;
    case DataType::Queries:
        refreshQueries();
        break;
    }
}

// Setup signal connections
void MainWindow::setupConnections()
{
    connect(ui->btnChangePassword, &QPushButton::clicked, this, &MainWindow::onChangePasswordClicked);

    // Study Planner UI Setup

    ui->addTaskButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->btnDeleteTask->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    ui->btnClearCompletedTasks->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    connect(ui->taskListWidget, &QListWidget::itemChanged, this, &MainWindow::on_taskItemChanged);
}

// Handle password change
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

// Refresh dashboard data
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
        // Ignore error if notices fail to load
    }
    for (const auto &n : notices)
    {
        ui->noticeListWidget->addItem("[" + n.getDate() + "] " + n.getAuthor() + ": " + n.getContent());
    }

    QString stats = myManager.getDashboardStats(userId, userRole);
    ui->label_welcome->setToolTip(stats);

    if (userRole == Constants::Role::Student)
    {
        std::unique_ptr<Student> s(myManager.getStudent(userId));
        if (s)
        {
            ui->val_p_name->setText(s->getName());
            ui->lbl_p_id->setVisible(true);
            ui->val_p_id->setVisible(true);
            ui->val_p_id->setText(QString::number(s->getId()));
            ui->val_p_dept->setText(s->getDepartment());
            ui->val_p_sem->setText(QString::number(s->getSemester()));
            ui->val_p_email->setText(s->getEmail());
        }
    }
    else if (userRole == Constants::Role::Teacher)
    {
        std::unique_ptr<Teacher> t(myManager.getTeacher(userId));
        if (t)
        {
            ui->val_p_name->setText(t->getName());
            ui->lbl_p_id->setVisible(false);
            ui->val_p_id->setVisible(false);
            ui->val_p_dept->setText(t->getDepartment());
            ui->lbl_p_sem->setText("Designation:");
            ui->val_p_sem->setText(t->getDesignation());
            ui->val_p_email->setText(t->getEmail());
        }
    }
    else if (userRole == Constants::Role::Admin)
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

// Post new notice
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
    }
}

// Logout and restart
void MainWindow::on_logoutButton_clicked()
{
    QApplication::exit(99);
}

// Refresh task list
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

// Add task
void MainWindow::on_addTaskButton_clicked()
{
    QString desc = ui->taskLineEdit->text();
    if (!desc.isEmpty())
    {
        myManager.addTask(userId, desc);
        ui->taskLineEdit->clear();
    }
}

// Delete selected task
void MainWindow::on_btnDeleteTask_clicked()
{
    QListWidgetItem *item = ui->taskListWidget->currentItem();
    if (item)
    {
        if (QMessageBox::question(this, "Delete Task", "Are you sure you want to delete this task?", QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        {
            return;
        }
        int id = item->data(Qt::UserRole).toInt();
        myManager.deleteTask(id);
        delete ui->taskListWidget->takeItem(ui->taskListWidget->row(item));
    }
}

// Clear completed tasks
void MainWindow::on_btnClearCompletedTasks_clicked()
{
    if (QMessageBox::question(this, "Clear Completed", "Are you sure you want to delete all completed tasks?", QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        myManager.deleteCompletedTasks(userId);
    }
}

// Update task status
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

// Focus Timer slots
void MainWindow::on_btnTimerStart_clicked()
{
    m_focusTimer->start(ui->spinTimerMinutes->value());
    ui->btnTimerPause->setText("Pause");
}

void MainWindow::on_btnTimerPause_clicked()
{
    m_focusTimer->pause();
    if (ui->btnTimerPause->text() == "Pause")
        ui->btnTimerPause->setText("Resume");
    else
        ui->btnTimerPause->setText("Pause");
}

void MainWindow::on_btnTimerStop_clicked()
{
    m_focusTimer->stop();
    ui->btnTimerPause->setText("Pause");
}

// Refresh habits and prayers
void MainWindow::refreshHabits()
{
    DailyPrayerStatus prayers = myManager.getDailyPrayers(userId, QDate::currentDate().toString(Qt::ISODate));
    ui->chkFajr->setChecked(prayers.getFajr());
    ui->chkDhuhr->setChecked(prayers.getDhuhr());
    ui->chkAsr->setChecked(prayers.getAsr());
    ui->chkMaghrib->setChecked(prayers.getMaghrib());
    ui->chkIsha->setChecked(prayers.getIsha());

    qDeleteAll(currentHabitList);
    activeTimerHabit = nullptr; // Prevent dangling pointer
    currentHabitList = myManager.getHabits(userId);
    ui->habitTableWidget->setRowCount(0);

    for (auto h : currentHabitList)
    {
        int row = ui->habitTableWidget->rowCount();
        ui->habitTableWidget->insertRow(row);

        ui->habitTableWidget->setItem(row, 0, new QTableWidgetItem(h->getTypeString()));
        ui->habitTableWidget->setItem(row, 1, new QTableWidgetItem(h->name));
        ui->habitTableWidget->setItem(row, 2, new QTableWidgetItem(h->getProgressString()));
        ui->habitTableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(h->streak)));
        QTableWidgetItem *statusItem = new QTableWidgetItem(h->isCompleted ? "Completed" : "Pending");
        if (h->isCompleted)
            statusItem->setForeground(Qt::green);
        ui->habitTableWidget->setItem(row, 4, statusItem);
    }
    ui->habitTableWidget->resizeColumnsToContents();
    ui->habitTableWidget->horizontalHeader()->setStretchLastSection(true);
}

// Create new habit
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

        QString unit = QInputDialog::getText(this, "Create Habit", "Unit:", QLineEdit::Normal, "", &ok);
        if (!ok)
            return;

        WorkoutHabit *h = new WorkoutHabit(0, userId, name, f, targetMin, targetCnt, unit);
        myManager.addHabit(h);
        delete h;
    }
}

// Perform selected habit
void MainWindow::on_btnPerformHabit_clicked()
{
    int row = ui->habitTableWidget->currentRow();
    if (row < 0 || row >= currentHabitList.size())
        return;

    Habit *h = currentHabitList[row];
    bool changed = false;

    if (auto wh = dynamic_cast<WorkoutHabit *>(h))
    {
        activeTimerHabit = wh;
        ui->groupBox_workoutTimer->setTitle("Workout: " + wh->name);

        double remaining = wh->targetMinutes - wh->currentMinutes;
        if (remaining <= 0.001)
            remaining = wh->targetMinutes;

        int totalSeconds = static_cast<int>(remaining * 60);
        ui->label_workoutTimerDisplay->setText(QString("%1:%2").arg(totalSeconds / 60, 2, 10, QChar('0')).arg(totalSeconds % 60, 2, 10, QChar('0')));

        QMessageBox::information(this, "Workout", "Timer set for '" + wh->name + "'.\nDon't forget to log your reps manually if needed!");
    }
    else if (auto dh = dynamic_cast<DurationHabit *>(h))
    {
        activeTimerHabit = dh;
        ui->groupBox_workoutTimer->setTitle("Stopwatch: " + dh->name);
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
    }
}

// Delete selected habit
void MainWindow::on_btnDeleteHabit_clicked()
{
    int row = ui->habitTableWidget->currentRow();
    if (row < 0 || row >= currentHabitList.size())
        return;

    int id = currentHabitList[row]->id;
    myManager.deleteHabit(id);
}

// Workout Timer slots
void MainWindow::on_btnWorkoutStart_clicked()
{
    if (!activeTimerHabit)
    {
        QMessageBox::warning(this, "Error", "Please select a habit to perform first.");
        return;
    }

    ui->btnWorkoutPause->setText("Pause");
    if (auto wh = dynamic_cast<WorkoutHabit *>(activeTimerHabit))
    {
        double remaining = wh->targetMinutes - wh->currentMinutes;
        if (remaining <= 0.001)
            remaining = wh->targetMinutes;
        m_workoutTimer->start(remaining);
    }
    else if (auto dh = dynamic_cast<DurationHabit *>(activeTimerHabit))
    {
        // Pure DurationHabit uses stopwatch
        double remaining = dh->targetMinutes - dh->currentMinutes;
        if (remaining <= 0.001)
            remaining = dh->targetMinutes;
        m_workoutTimer->startStopwatch(remaining);
    }
}

void MainWindow::on_btnWorkoutPause_clicked()
{
    m_workoutTimer->pause();
    if (ui->btnWorkoutPause->text() == "Pause")
        ui->btnWorkoutPause->setText("Resume");
    else
        ui->btnWorkoutPause->setText("Pause");
}

void MainWindow::on_btnWorkoutStop_clicked()
{
    double elapsed = m_workoutTimer->getElapsedMinutes();
    m_workoutTimer->stop();
    ui->btnWorkoutPause->setText("Pause");

    if (activeTimerHabit)
    {
        if (elapsed > 0)
        {
            activeTimerHabit->currentMinutes += elapsed;

            if (auto wh = dynamic_cast<WorkoutHabit *>(activeTimerHabit))
            {
                bool ok;
                int count = QInputDialog::getInt(this, "Workout Stopped", "Add Reps/Count:", 0, 0, 1000, 1, &ok);
                if (ok && count > 0)
                    wh->currentCount += count;

                if (wh->currentMinutes >= wh->targetMinutes && wh->currentCount >= wh->targetCount)
                    wh->markComplete();
            }
            else if (activeTimerHabit->currentMinutes >= activeTimerHabit->targetMinutes)
                activeTimerHabit->markComplete();

            myManager.updateHabit(activeTimerHabit);
        }
    }
}

// Prayer toggle slots
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

// Refresh student routine
void MainWindow::refreshRoutine()
{
    ui->tableRoutine->setRowCount(0);
    QString day = ui->comboRoutineDay->currentText();

    int semester = -1;
    if (userRole == Constants::Role::Student)
    {
        std::unique_ptr<Student> s(myManager.getStudent(userId));
        if (s)
        {
            semester = s->getSemester();
        }
    }

    QVector<RoutineSession> items = myManager.getEffectiveRoutine(Utils::getDateForDay(day), semester);

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

// Refresh teacher routine
void MainWindow::refreshTeacherRoutine()
{
    ui->tableTeacherRoutine->setRowCount(0);
    QString day = ui->comboRoutineDayInput->currentText();
    QVector<RoutineSession> items = myManager.getEffectiveRoutine(Utils::getDateForDay(day));

    if (userRole == Constants::Role::Teacher)
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

// Cancel class
void MainWindow::on_btnCancelClass_clicked()
{
    int row = ui->tableTeacherRoutine->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(this, "Selection", "Please select a class to cancel.");
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
        QMessageBox::information(this, "Success", "Class cancelled.");
    }
}

// Reschedule class
void MainWindow::on_btnRescheduleClass_clicked()
{
    int row = ui->tableTeacherRoutine->currentRow();
    if (row < 0)
    {
        QMessageBox::warning(this, "Selection", "Please select a class to exchange/reschedule.");
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

    // Delegate logic to Manager (Controller)
    QVector<RescheduleOption> optionsList = myManager.getRescheduleOptions(originDate, originSerial, semester, originCode, originRoom, userName);

    QStringList options;
    for (const auto &opt : optionsList)
    {
        options << opt.displayText;
    }

    bool ok;
    QString choice = QInputDialog::getItem(this, "Select New Slot", "Available Options (Next 2 Weeks):", options, 0, false, &ok);
    if (ok && !choice.isEmpty())
    {
        int idx = options.indexOf(choice);
        if (idx >= 0 && idx < optionsList.size())
        {
            // Apply the adjustment
            myManager.addRoutineAdjustment(optionsList[idx].adjustment);

            if (!optionsList[idx].secondaryAdjustment.courseCode.isEmpty())
            {
                myManager.addRoutineAdjustment(optionsList[idx].secondaryAdjustment);
            }

            QMessageBox::information(this, "Success", "Class rescheduled/exchanged successfully.");
        }
    }
}

// Refresh academic data
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

// Refresh teacher tools
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

// Create assessment
void MainWindow::on_btnCreateAssessment_clicked()
{
    int courseId = ui->comboTeacherCourse->currentData().toInt();
    QString title = ui->editAssessmentTitle->text();
    if (title.isEmpty())
        return;

    myManager.addAssessment(courseId, title, ui->comboAssessmentType->currentText(),
                            QDate::currentDate().toString("yyyy-MM-dd"), ui->spinMaxMarks->value());
    ui->editAssessmentTitle->clear();
    QMessageBox::information(this, "Success", "Assessment Created");
}

void MainWindow::on_comboTeacherAssessment_currentIndexChanged(int index)
{
    refreshTeacherGrades();
}

// Refresh grades table
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

    std::unique_ptr<Course> c(myManager.getCourse(courseId));

    QVector<Student *> students;
    if (c)
    {
        students = myManager.getStudentsBySemester(c->getSemester());
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

// Save grades
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
}

// Refresh attendance table
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
    std::unique_ptr<Course> c(myManager.getCourse(courseId));
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
    Utils::adjustColumnWidths(ui->tableAttendance);
}

void MainWindow::on_comboAttendanceCourse_currentIndexChanged(int index)
{
    refreshTeacherAttendance();
}

// Save attendance
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
}

// Refresh queries
void MainWindow::refreshQueries()
{
    ui->listQueries->clear();
    QVector<Query> queries = myManager.getQueries(userId, userRole);

    for (const auto &q : queries)
    {
        QString label;
        if (userRole == Constants::Role::Student)
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

// Handle query action
void MainWindow::on_btnQueryAction_clicked()
{
    QString text = ui->editQueryInput->text();
    if (text.isEmpty())
        return;

    if (userRole == Constants::Role::Student)
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
    }
}

// Load admin table
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
    if (tableName == Constants::Table::Admins)
        headers << "ID" << "Username" << "Password" << "Name" << "Email";
    else if (tableName == Constants::Table::Students)
        headers << "ID" << "Name" << "Email" << "Username" << "Password" << "Dept" << "Batch" << "Sem" << "Admission Date" << "CGPA";
    else if (tableName == Constants::Table::Teachers)
        headers << "ID" << "Name" << "Email" << "Username" << "Password" << "Dept" << "Designation" << "Salary";
    else if (tableName == Constants::Table::Courses)
        headers << "ID" << "Code" << "Name" << "Teacher ID" << "Semester" << "Credits";
    else if (tableName == Constants::Table::Routine)
        headers << "Day" << "Serial" << "Code" << "Name" << "Room" << "Instructor" << "Semester";
    else if (tableName == Constants::Table::RoutineAdj)
        headers << "Orig Date" << "Orig Serial" << "Type" << "New Date" << "New Serial" << "Code" << "Name" << "Room" << "Instructor" << "Semester";
    else if (tableName == "grades")
        headers << "Student ID" << "Assessment ID" << "Marks";
    else if (tableName == "notices")
        headers << "Date" << "Author" << "Content";
    adminModel->setHorizontalHeaderLabels(headers);

    connect(adminModel, &QStandardItemModel::itemChanged, this, [this, tableName]()
            { Utils::saveTableData(adminModel, tableName); });

    QTimer::singleShot(10, this, [this]()
                       { Utils::adjustColumnWidths(ui->adminTableView); });
}

// Add admin row
void MainWindow::on_btnAddRow_clicked()
{
    int row = adminModel->rowCount();
    adminModel->insertRow(row);

    QString currentTable = ui->tableComboBox->currentText();
    QString nextIdStr = "";

    if (currentTable == Constants::Table::Admins || currentTable == Constants::Table::Students ||
        currentTable == Constants::Table::Teachers || currentTable == Constants::Table::Courses)
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

    Utils::saveTableData(adminModel, currentTable);
}

// Delete admin row
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

    Utils::saveTableData(adminModel, ui->tableComboBox->currentText());
}

// Filter admin table
void MainWindow::on_searchLineEdit_textChanged(const QString &arg1)
{
    adminProxyModel->setFilterFixedString(arg1);
}

// Handle resize event
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
