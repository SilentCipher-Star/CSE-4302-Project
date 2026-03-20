#include "../include/mainwindow.hpp"
#include "../include/theme.hpp"
#include "../include/assetmanager.hpp"
#include <QApplication>
#include "../include/ui_dashboard.hpp"
#include "../include/ui_planner.hpp"
#include "../include/ui_habits.hpp"
#include "../include/ui_timers.hpp"
#include "../include/ui_routine.hpp"
#include "../include/ui_academics.hpp"
#include "../include/ui_tamagotchi.hpp"
#include "../include/ui_calendar.hpp"
#include "../include/ui_lostfound.hpp"
#include "../include/ui_queries.hpp"
#include "../include/ui_admin.hpp"
#include "../include/utils.hpp"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QGraphicsDropShadowEffect>
#include <QHeaderView>
#include <QResizeEvent>
#include <QTimer>
#include <QDate>
#include <QSettings>
#include <QShortcut>
#include <QProgressBar>
#include <exception>

MainWindow::MainWindow(QString role, int uid, QString name, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), userRole(role), userId(uid), userName(name)
{
    myManager.addObserver(this);

    ui->setupUi(this);

    // Setup container for corner widgets like theme toggle and logout
    QWidget *cornerContainer = new QWidget(this);
    QHBoxLayout *cornerLayout = new QHBoxLayout(cornerContainer);
    cornerLayout->setContentsMargins(0, 0, 6, 0);
    cornerLayout->setSpacing(6);

    // Retrieve persisted theme settings from storage
    QSettings settings("Acadence", "Acadence");
    m_darkMode = settings.value("theme/darkMode", false).toBool();
    m_darkThemeIndex = settings.value("theme/darkIndex", 0).toInt();
    QString savedName = settings.value("theme/name", "").toString();

    auto themes = ThemeManager::getAvailableThemes();
    if (!themes.isEmpty())
        m_userTheme = themes[0];
    for (const auto &t : themes)
    {
        if (t.name.trimmed() == savedName.trimmed())
        {
            m_userTheme = t;
            break;
        }
    }

    // Apply the loaded theme immediately on startup
    AppTheme currentTheme;
    if (m_darkMode)
    {
        auto dark = ThemeManager::getDarkThemes();
        m_darkThemeIndex = qBound(0, m_darkThemeIndex, dark.size() - 1);
        currentTheme = dark[m_darkThemeIndex];
        ThemeManager::applyTheme(*static_cast<QApplication *>(QApplication::instance()), currentTheme);
    }
    else
    {
        currentTheme = m_userTheme;
        ThemeManager::applyTheme(*static_cast<QApplication *>(QApplication::instance()), m_userTheme);
    }

    // Initialize global undo and redo buttons
    m_undoBtn = new QPushButton("Undo", cornerContainer);
    m_undoBtn->setToolTip("Nothing to undo");
    m_undoBtn->setEnabled(false);
    connect(m_undoBtn, &QPushButton::clicked, this, [this]()
            {
        myManager.undo();
        updateUndoRedoButtons(); });

    m_redoBtn = new QPushButton("Redo", cornerContainer);
    m_redoBtn->setToolTip("Nothing to redo");
    m_redoBtn->setEnabled(false);
    connect(m_redoBtn, &QPushButton::clicked, this, [this]()
            {
        myManager.redo();
        updateUndoRedoButtons(); });

    // Bind standard keyboard shortcuts for undo and redo
    QShortcut *undoShortcut = new QShortcut(QKeySequence::Undo, this);
    connect(undoShortcut, &QShortcut::activated, m_undoBtn, &QPushButton::click);

    QShortcut *redoShortcut = new QShortcut(QKeySequence::Redo, this);
    connect(redoShortcut, &QShortcut::activated, m_redoBtn, &QPushButton::click);

    // Setup theme toggling widgets
    m_themeBtn = new QPushButton(m_darkMode ? "Dark Theme" : "Theme", cornerContainer);
    connect(m_themeBtn, &QPushButton::clicked, this, &MainWindow::onThemeClicked);

    m_toggle = new ThemeToggle(cornerContainer);
    m_toggle->setDark(m_darkMode);
    connect(m_toggle, &ThemeToggle::toggled, this, &MainWindow::toggleDarkMode);

    cornerLayout->addWidget(m_undoBtn);
    cornerLayout->addWidget(m_redoBtn);
    cornerLayout->addWidget(m_themeBtn);
    cornerLayout->addWidget(m_toggle);
    cornerLayout->addWidget(ui->logoutButton);
    ui->tabWidget->setCornerWidget(cornerContainer, Qt::TopRightCorner);

    // Configure main window flags and initial maximization state
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowState(Qt::WindowMaximized);

    ui->label_welcome->setText("Welcome, " + name);

    // Add decorative shadow effect to the profile group box
    QGraphicsDropShadowEffect *profileShadow = new QGraphicsDropShadowEffect(ui->groupBox_profile);
    profileShadow->setBlurRadius(20);
    profileShadow->setXOffset(0);
    profileShadow->setYOffset(5);
    profileShadow->setColor(QColor(0, 0, 0, 40));
    ui->groupBox_profile->setGraphicsEffect(profileShadow);

    // Instantiate main UI modules
    uiDashboard = new UIDashboard(ui, &myManager, role, uid, name, this);
    uiPlanner = new UIPlanner(ui, &myManager, uid, this);
    uiTimers = new UITimers(ui, &myManager, uid, this);
    uiHabits = new UIHabits(ui, &myManager, uid, uiTimers->getWorkoutTimer(), this);
    uiRoutine = new UIRoutine(ui, &myManager, role, uid, name, this);
    uiAcademics = new UIAcademics(ui, &myManager, role, uid, this);
    uiQueries = new UIQueries(ui, &myManager, role, uid, this);
    uiAdmin = new UIAdmin(ui, &myManager, this);

    // Link habits and timers to allow integrated session tracking
    uiTimers->setHabitsModule(uiHabits);

    // Prepare tables and timer defaults
    setupTables();
    uiTimers->setupTimers();
    setupConnections();

    // Append universal Lost & Found module
    uiLostFound = new UILostFound(&myManager, role, uid, name, this);
    ui->tabWidget->addTab(uiLostFound->getWidget(), "Lost && Found");

    // Adjust visible tabs and capabilities based on user role
    if (role == Constants::Role::Student)
    {
        uiTamagotchi = new UITamagotchi(&myManager, role, uid, this);
        ui->tabWidget->addTab(uiTamagotchi->getWidget(), "Tamagotchi");

        uiCalendar = new UICalendar(&myManager, role, uid, this);
        ui->tabWidget->addTab(uiCalendar->getWidget(), "Calendar");

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
        ui->addNoticeButton->setText("Post Notice");
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
        ui->addNoticeButton->setText("Post Targeted Notice");
    }

    // Show loading overlay while initial data fetch occurs
    m_loadingOverlay = new QWidget(this);
    m_loadingOverlay->setObjectName("loadingOverlay");

    QColor bgColor(currentTheme.background);
    bgColor.setAlpha(245);
    QString bgRgba = QString("rgba(%1, %2, %3, %4)").arg(bgColor.red()).arg(bgColor.green()).arg(bgColor.blue()).arg(bgColor.alpha());
    m_loadingOverlay->setStyleSheet(QString("background-color: %1; color: %2;").arg(bgRgba, currentTheme.text));

    QVBoxLayout *overlayLayout = new QVBoxLayout(m_loadingOverlay);
    overlayLayout->setAlignment(Qt::AlignCenter);

    QLabel *lblLoading = new QLabel("Loading Acadence...", m_loadingOverlay);
    lblLoading->setStyleSheet(QString("font-size: %1px; font-weight: bold; background: transparent; color: %2;").arg(AppFonts::Large).arg(currentTheme.text));
    overlayLayout->addWidget(lblLoading);

    QProgressBar *loadingBar = new QProgressBar(m_loadingOverlay);
    loadingBar->setRange(0, 0); // Indeterminate
    loadingBar->setFixedWidth(250);
    loadingBar->setTextVisible(false);
    QString barStyle = QString("QProgressBar { border: none; background: %1; height: 6px; border-radius: 3px; } QProgressBar::chunk { background: %2; border-radius: 3px; }")
                           .arg(currentTheme.surface, currentTheme.accent);
    loadingBar->setStyleSheet(barStyle);

    overlayLayout->addSpacing(20);
    overlayLayout->addWidget(loadingBar);
    m_loadingOverlay->resize(width(), height());
    m_loadingOverlay->show();

    // Defer data loading slightly to allow window to render
    QTimer::singleShot(100, this, [this, role]()
                       {
        try
        {
            if (role == Constants::Role::Teacher)
            {
                uiRoutine->refreshTeacherRoutine();
                uiAcademics->refreshTeacherTools();
            }
            else if (role == Constants::Role::Admin)
            {
                uiAdmin->initialize();
            }

            uiDashboard->refreshDashboard();
            uiQueries->refreshQueries();
            if (role == Constants::Role::Student)
            {
                uiPlanner->refreshPlanner();
                uiHabits->refreshHabits();

                int currentDayIndex = QDate::currentDate().dayOfWeek() % 7;
                ui->comboRoutineDay->setCurrentIndex(currentDayIndex);

                uiAcademics->refreshAcademics();
            }
        }
        catch (const Acadence::Exception &e)
        {
            QMessageBox::critical(this, "Data Error", QString("Failed to load initial data:\n%1").arg(e.what()));
        }
        catch (const std::exception &e)
        {
            QMessageBox::critical(this, "Data Error", QString("Standard exception during load:\n%1").arg(e.what()));
        }
        catch (...)
        {
            QMessageBox::critical(this, "Data Error", "Unknown error during initial data load.");
        }
        
        if (m_loadingOverlay)
        {
            m_loadingOverlay->deleteLater();
            m_loadingOverlay = nullptr;
        } });
}

MainWindow::~MainWindow()
{
    myManager.removeObserver(this);
    delete ui;
}

void MainWindow::setupTables()
{
    // Make key tables interactive and prevent unwanted stretching
    QList<QTableView *> tables = {
        ui->adminTableView, ui->habitTableWidget, ui->tableRoutine, ui->tableTeacherRoutine,
        ui->tableAcademics, ui->tableGrading, ui->tableAttendance};

    for (auto table : tables)
    {
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        table->horizontalHeader()->setStretchLastSection(false);
    }
}

void MainWindow::onDataChanged(DataType type)
{
    switch (type)
    {
    case DataType::Habits:
        uiHabits->refreshHabits();
        if (userRole == Constants::Role::Student && uiTamagotchi)
            uiTamagotchi->refreshTamagotchi();
        break;
    case DataType::Tasks:
        uiPlanner->refreshPlanner();
        uiDashboard->refreshDashboard();
        break;
    case DataType::Routine:
        if (userRole == Constants::Role::Teacher)
            uiRoutine->refreshTeacherRoutine();
        else
            uiRoutine->refreshRoutine();
        uiDashboard->refreshDashboard();
        if (userRole == Constants::Role::Student && uiCalendar)
            uiCalendar->refresh();
        break;
    case DataType::Notices:
        uiDashboard->refreshDashboard();
        break;
    case DataType::Academics:
        uiAcademics->refreshAcademics();
        if (userRole == Constants::Role::Teacher)
            uiAcademics->refreshTeacherTools();
        uiDashboard->refreshDashboard();
        if (userRole == Constants::Role::Student && uiTamagotchi)
            uiTamagotchi->refreshTamagotchi();
        break;
    case DataType::Queries:
        uiQueries->refreshQueries();
        break;
    case DataType::Profile:
        uiDashboard->refreshDashboard();
        if (userRole == Constants::Role::Student && uiTamagotchi)
            uiTamagotchi->refreshTamagotchi();
        break;
    case DataType::None:
        break;
    case DataType::Messages:
        break;
    case DataType::LostFound:
        if (uiLostFound)
            uiLostFound->refresh();
        break;
    }
    updateUndoRedoButtons();
}

void MainWindow::updateUndoRedoButtons()
{
    // Sync undo/redo button states with command history
    m_undoBtn->setEnabled(myManager.canUndo());
    m_redoBtn->setEnabled(myManager.canRedo());
    m_undoBtn->setToolTip(myManager.canUndo()
                              ? "Undo: " + myManager.undoDescription()
                              : "Nothing to undo");
    m_redoBtn->setToolTip(myManager.canRedo()
                              ? "Redo: " + myManager.redoDescription()
                              : "Nothing to redo");
}

void MainWindow::toggleDarkMode(bool isDark)
{
    // Switch global theme between light and dark mode variants
    m_darkMode = isDark;

    QSettings settings("Acadence", "Acadence");
    settings.setValue("theme/darkMode", m_darkMode);

    if (m_darkMode)
    {
        auto dark = ThemeManager::getDarkThemes();
        m_darkThemeIndex = qBound(0, m_darkThemeIndex, dark.size() - 1);
        ThemeManager::applyTheme(*static_cast<QApplication *>(QApplication::instance()),
                                 dark[m_darkThemeIndex]);
        m_themeBtn->setText("Dark Theme");
        settings.setValue("theme/darkIndex", m_darkThemeIndex);
    }
    else
    {
        ThemeManager::applyTheme(*static_cast<QApplication *>(QApplication::instance()),
                                 m_userTheme);
        m_themeBtn->setText("Theme");
        settings.setValue("theme/name", m_userTheme.name.trimmed());
    }
}

void MainWindow::onThemeClicked()
{
    // Cycle through available themes of the current mode
    QSettings settings("Acadence", "Acadence");

    if (m_darkMode)
    {
        // Move to the next dark theme
        auto dark = ThemeManager::getDarkThemes();
        m_darkThemeIndex = (m_darkThemeIndex + 1) % dark.size();
        ThemeManager::applyTheme(*static_cast<QApplication *>(QApplication::instance()),
                                 dark[m_darkThemeIndex]);
        settings.setValue("theme/darkIndex", m_darkThemeIndex);
        return;
    }

    // Move to the next light theme
    auto themes = ThemeManager::getAvailableThemes();
    int idx = -1;
    for (int i = 0; i < themes.size(); ++i)
    {
        if (themes[i].name.trimmed() == m_userTheme.name.trimmed())
        {
            idx = i;
            break;
        }
    }
    int nextIdx = (idx + 1) % themes.size();
    m_userTheme = themes[nextIdx];
    ThemeManager::applyTheme(*static_cast<QApplication *>(QApplication::instance()), m_userTheme);
    settings.setValue("theme/name", m_userTheme.name.trimmed());
}

void MainWindow::setupConnections()
{
    uiPlanner->setupConnections();

    // Connect UI actions for Dashboard
    connect(ui->btnChangePassword, &QPushButton::clicked, uiDashboard, &UIDashboard::onChangePasswordClicked);
    connect(ui->addNoticeButton, &QPushButton::clicked, uiDashboard, &UIDashboard::onAddNoticeClicked);
    connect(ui->logoutButton, &QPushButton::clicked, uiDashboard, &UIDashboard::onLogoutClicked);
    connect(ui->noticeListWidget, &QListWidget::itemClicked, uiDashboard, &UIDashboard::onNoticeItemClicked);
    connect(ui->noticeListWidget, &QListWidget::itemDoubleClicked, uiDashboard, &UIDashboard::onNoticeItemDoubleClicked);

    // Connect UI actions for Study Planner
    connect(ui->addTaskButton, &QPushButton::clicked, uiPlanner, &UIPlanner::onAddTaskClicked);
    connect(ui->btnDeleteTask, &QPushButton::clicked, uiPlanner, &UIPlanner::onDeleteTaskClicked);
    connect(ui->btnClearCompletedTasks, &QPushButton::clicked, uiPlanner, &UIPlanner::onClearCompletedTasksClicked);
    connect(ui->taskListWidget, &QListWidget::itemChanged, uiPlanner, &UIPlanner::onTaskItemChanged);

    // Connect UI actions for Focus Timer
    connect(ui->btnTimerStart, &QPushButton::clicked, uiTimers, &UITimers::onFocusTimerStartClicked);
    connect(ui->btnTimerPause, &QPushButton::clicked, uiTimers, &UITimers::onFocusTimerPauseClicked);
    connect(ui->btnTimerStop, &QPushButton::clicked, uiTimers, &UITimers::onFocusTimerStopClicked);

    // Connect UI actions for Workout Timer
    connect(ui->btnWorkoutStart, &QPushButton::clicked, uiTimers, &UITimers::onWorkoutTimerStartClicked);
    connect(ui->btnWorkoutPause, &QPushButton::clicked, uiTimers, &UITimers::onWorkoutTimerPauseClicked);
    connect(ui->btnWorkoutStop, &QPushButton::clicked, uiTimers, &UITimers::onWorkoutTimerStopClicked);

    // Connect UI actions for Habits management
    connect(ui->btnAddHabit, &QPushButton::clicked, uiHabits, &UIHabits::onAddHabitClicked);
    connect(ui->btnPerformHabit, &QPushButton::clicked, uiHabits, &UIHabits::onPerformHabitClicked);
    connect(ui->btnDeleteHabit, &QPushButton::clicked, uiHabits, &UIHabits::onDeleteHabitClicked);

    // Connect UI actions for Prayer tracker
    connect(ui->chkFajr, &QCheckBox::toggled, uiHabits, &UIHabits::onFajrToggled);
    connect(ui->chkDhuhr, &QCheckBox::toggled, uiHabits, &UIHabits::onDhuhrToggled);
    connect(ui->chkAsr, &QCheckBox::toggled, uiHabits, &UIHabits::onAsrToggled);
    connect(ui->chkMaghrib, &QCheckBox::toggled, uiHabits, &UIHabits::onMaghribToggled);
    connect(ui->chkIsha, &QCheckBox::toggled, uiHabits, &UIHabits::onIshaToggled);
    connect(ui->btnPrayerHistory, &QPushButton::clicked, uiHabits, &UIHabits::onPrayerHistoryClicked);

    // Connect UI actions for Routine view
    connect(ui->comboRoutineDay, QOverload<int>::of(&QComboBox::currentIndexChanged), uiRoutine, &UIRoutine::onRoutineDayChanged);
    connect(ui->comboRoutineDayInput, QOverload<int>::of(&QComboBox::currentIndexChanged), uiRoutine, &UIRoutine::onTeacherRoutineDayChanged);
    connect(ui->btnCancelClass, &QPushButton::clicked, uiRoutine, &UIRoutine::onCancelClassClicked);
    connect(ui->btnRescheduleClass, &QPushButton::clicked, uiRoutine, &UIRoutine::onRescheduleClassClicked);

    // Connect UI actions for Teacher Academic Tools
    connect(ui->btnCreateAssessment, &QPushButton::clicked, uiAcademics, &UIAcademics::onCreateAssessmentClicked);
    connect(ui->comboTeacherAssessment, QOverload<int>::of(&QComboBox::currentIndexChanged), uiAcademics, &UIAcademics::onTeacherAssessmentChanged);
    connect(ui->btnSaveGrades, &QPushButton::clicked, uiAcademics, &UIAcademics::onSaveGradesClicked);
    connect(ui->comboAttendanceCourse, QOverload<int>::of(&QComboBox::currentIndexChanged), uiAcademics, &UIAcademics::onAttendanceCourseChanged);
    connect(ui->btnAddClassDate, &QPushButton::clicked, uiAcademics, &UIAcademics::onAddClassDateClicked);
    connect(ui->btnSaveAttendance, &QPushButton::clicked, uiAcademics, &UIAcademics::onSaveAttendanceClicked);

    // Connect UI actions for Q&A section
    connect(ui->btnQueryAction, &QPushButton::clicked, uiQueries, &UIQueries::onQueryActionClicked);

    // Connect UI actions for Admin operations
    connect(ui->tableComboBox, &QComboBox::currentTextChanged, uiAdmin, &UIAdmin::onTableComboBoxChanged);
    connect(ui->btnAddRow, &QPushButton::clicked, uiAdmin, &UIAdmin::onAddRowClicked);
    connect(ui->btnDeleteRow, &QPushButton::clicked, uiAdmin, &UIAdmin::onDeleteRowClicked);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, uiAdmin, &UIAdmin::onSearchTextChanged);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (m_loadingOverlay)
        m_loadingOverlay->resize(size());

    // Adjust active table column widths when the window is resized
    QTimer::singleShot(0, this, [=]()
                       {
        if (ui->adminTableView->isVisible())
            ui->adminTableView->adjustColumnWidths();
        if (ui->habitTableWidget->isVisible())
            ui->habitTableWidget->adjustColumnWidths();
        if (ui->tableRoutine->isVisible())
            ui->tableRoutine->adjustColumnWidths();
        if (ui->tableTeacherRoutine->isVisible())
            ui->tableTeacherRoutine->adjustColumnWidths();
        if (ui->tableAttendance->isVisible())
            ui->tableAttendance->adjustColumnWidths(); });
}
