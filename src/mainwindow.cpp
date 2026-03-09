#include "../include/mainwindow.hpp"
#include "../include/theme.hpp"
#include <QApplication>
#include "../include/ui_dashboard.hpp"
#include "../include/ui_planner.hpp"
#include "../include/ui_habits.hpp"
#include "../include/ui_timers.hpp"
#include "../include/ui_routine.hpp"
#include "../include/ui_academics.hpp"
#include "../include/ui_tamagotchi.hpp"
#include "../include/ui_calendar.hpp"
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

MainWindow::MainWindow(QString role, int uid, QString name, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), userRole(role), userId(uid), userName(name)
{
    myManager.addObserver(this);

    ui->setupUi(this);

    // Corner widget: Dark Mode toggle + Logout
    QWidget *cornerContainer = new QWidget(this);
    QHBoxLayout *cornerLayout = new QHBoxLayout(cornerContainer);
    cornerLayout->setContentsMargins(0, 0, 6, 0);
    cornerLayout->setSpacing(6);

    // Detect the current theme applied from LoginDialog
    auto themes = ThemeManager::getAvailableThemes();
    if (!themes.isEmpty())
        m_userTheme = themes[0]; // Default fallback

    QString currentBg = QApplication::palette().color(QPalette::Window).name(QColor::HexRgb);
    for (const auto &t : themes)
    {
        if (QColor(t.background).name(QColor::HexRgb) == currentBg)
        {
            m_userTheme = t;
            break;
        }
    }

    m_themeBtn = new QPushButton("Theme", cornerContainer);
    connect(m_themeBtn, &QPushButton::clicked, this, &MainWindow::onThemeClicked);

    m_darkModeBtn = new QPushButton("Dark Mode", cornerContainer);
    connect(m_darkModeBtn, &QPushButton::clicked, this, &MainWindow::toggleDarkMode);

    cornerLayout->addWidget(m_themeBtn);
    cornerLayout->addWidget(m_darkModeBtn);
    cornerLayout->addWidget(ui->logoutButton);
    ui->tabWidget->setCornerWidget(cornerContainer, Qt::TopRightCorner);

    // Configure window state
    setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    setWindowState(Qt::WindowMaximized);

    ui->label_welcome->setText("Welcome, " + name);

    // Apply shadow effect
    QGraphicsDropShadowEffect *profileShadow = new QGraphicsDropShadowEffect(ui->groupBox_profile);
    profileShadow->setBlurRadius(20);
    profileShadow->setXOffset(0);
    profileShadow->setYOffset(5);
    profileShadow->setColor(QColor(0, 0, 0, 40));
    ui->groupBox_profile->setGraphicsEffect(profileShadow);

    // Initialize UI modules
    uiDashboard = new UIDashboard(ui, &myManager, role, uid, name, this);
    uiPlanner = new UIPlanner(ui, &myManager, uid, this);
    uiTimers = new UITimers(ui, &myManager, uid, this);
    uiHabits = new UIHabits(ui, &myManager, uid, uiTimers->getWorkoutTimer(), this);
    uiRoutine = new UIRoutine(ui, &myManager, role, uid, name, this);
    uiAcademics = new UIAcademics(ui, &myManager, role, uid, this);
    uiQueries = new UIQueries(ui, &myManager, role, uid, this);
    uiAdmin = new UIAdmin(ui, &myManager, this);

    // Link habits module to timers
    uiTimers->setHabitsModule(uiHabits);

    // Setup UI
    setupTables();
    uiTimers->setupTimers();
    setupConnections();

    // Configure UI based on role
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
        uiRoutine->refreshTeacherRoutine();
        uiAcademics->refreshTeacherTools();
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
        uiAdmin->initialize();
    }

    // Load data
    try
    {
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
}

MainWindow::~MainWindow()
{
    myManager.removeObserver(this);
    delete ui;
}

// Configure table headers
void MainWindow::setupTables()
{
    QList<QTableView *> tables = {
        ui->adminTableView, ui->tableRoutine, ui->tableTeacherRoutine,
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
        break;
    case DataType::Tasks:
        uiPlanner->refreshPlanner();
        break;
    case DataType::Routine:
        if (userRole == Constants::Role::Teacher)
            uiRoutine->refreshTeacherRoutine();
        else
            uiRoutine->refreshRoutine();
        break;
    case DataType::Notices:
        uiDashboard->refreshDashboard();
        break;
    case DataType::Academics:
        uiAcademics->refreshAcademics();
        if (userRole == Constants::Role::Teacher)
            uiAcademics->refreshTeacherTools();
        break;
    case DataType::Queries:
        uiQueries->refreshQueries();
        break;
    case DataType::Profile:
        uiDashboard->refreshDashboard();
        break;
    case DataType::None:
        break;
    }
}

void MainWindow::toggleDarkMode()
{
    m_darkMode = !m_darkMode;

    AppTheme theme;
    if (m_darkMode)
    {
        theme = {"Midnight", "#1a1b2e", "#252641", "#e2e8f0", "#818cf8"};
        m_darkModeBtn->setText("Light Mode");
        m_themeBtn->setEnabled(false); // Disable theme cycling in dark mode
    }
    else
    {
        theme = m_userTheme;
        m_darkModeBtn->setText("Dark Mode");
        m_themeBtn->setEnabled(true);
    }

    ThemeManager::applyTheme(*static_cast<QApplication *>(QApplication::instance()), theme);
}

void MainWindow::onThemeClicked()
{
    if (m_darkMode)
        return;

    auto themes = ThemeManager::getAvailableThemes();
    int idx = -1;
    for (int i = 0; i < themes.size(); ++i)
    {
        if (themes[i].name == m_userTheme.name)
        {
            idx = i;
            break;
        }
    }
    int nextIdx = (idx + 1) % themes.size();
    m_userTheme = themes[nextIdx];
    ThemeManager::applyTheme(*static_cast<QApplication *>(QApplication::instance()), m_userTheme);
}

void MainWindow::setupConnections()
{
    uiPlanner->setupConnections();

    // Dashboard connections
    connect(ui->btnChangePassword, &QPushButton::clicked, uiDashboard, &UIDashboard::onChangePasswordClicked);
    connect(ui->addNoticeButton, &QPushButton::clicked, uiDashboard, &UIDashboard::onAddNoticeClicked);
    connect(ui->logoutButton, &QPushButton::clicked, uiDashboard, &UIDashboard::onLogoutClicked);
    connect(ui->noticeListWidget, &QListWidget::itemClicked, uiDashboard, &UIDashboard::onNoticeItemClicked);
    connect(ui->noticeListWidget, &QListWidget::itemDoubleClicked, uiDashboard, &UIDashboard::onNoticeItemDoubleClicked);

    // Study Planner connections
    connect(ui->addTaskButton, &QPushButton::clicked, uiPlanner, &UIPlanner::onAddTaskClicked);
    connect(ui->btnDeleteTask, &QPushButton::clicked, uiPlanner, &UIPlanner::onDeleteTaskClicked);
    connect(ui->btnClearCompletedTasks, &QPushButton::clicked, uiPlanner, &UIPlanner::onClearCompletedTasksClicked);
    connect(ui->taskListWidget, &QListWidget::itemChanged, uiPlanner, &UIPlanner::onTaskItemChanged);

    // Focus Timer connections
    connect(ui->btnTimerStart, &QPushButton::clicked, uiTimers, &UITimers::onFocusTimerStartClicked);
    connect(ui->btnTimerPause, &QPushButton::clicked, uiTimers, &UITimers::onFocusTimerPauseClicked);
    connect(ui->btnTimerStop, &QPushButton::clicked, uiTimers, &UITimers::onFocusTimerStopClicked);

    // Workout Timer connections
    connect(ui->btnWorkoutStart, &QPushButton::clicked, uiTimers, &UITimers::onWorkoutTimerStartClicked);
    connect(ui->btnWorkoutPause, &QPushButton::clicked, uiTimers, &UITimers::onWorkoutTimerPauseClicked);
    connect(ui->btnWorkoutStop, &QPushButton::clicked, uiTimers, &UITimers::onWorkoutTimerStopClicked);

    // Habits connections
    connect(ui->btnAddHabit, &QPushButton::clicked, uiHabits, &UIHabits::onAddHabitClicked);
    connect(ui->btnPerformHabit, &QPushButton::clicked, uiHabits, &UIHabits::onPerformHabitClicked);
    connect(ui->btnDeleteHabit, &QPushButton::clicked, uiHabits, &UIHabits::onDeleteHabitClicked);

    // Prayer connections
    connect(ui->chkFajr, &QCheckBox::toggled, uiHabits, &UIHabits::onFajrToggled);
    connect(ui->chkDhuhr, &QCheckBox::toggled, uiHabits, &UIHabits::onDhuhrToggled);
    connect(ui->chkAsr, &QCheckBox::toggled, uiHabits, &UIHabits::onAsrToggled);
    connect(ui->chkMaghrib, &QCheckBox::toggled, uiHabits, &UIHabits::onMaghribToggled);
    connect(ui->chkIsha, &QCheckBox::toggled, uiHabits, &UIHabits::onIshaToggled);

    // Routine connections
    connect(ui->comboRoutineDay, QOverload<int>::of(&QComboBox::currentIndexChanged), uiRoutine, &UIRoutine::onRoutineDayChanged);
    connect(ui->comboRoutineDayInput, QOverload<int>::of(&QComboBox::currentIndexChanged), uiRoutine, &UIRoutine::onTeacherRoutineDayChanged);
    connect(ui->btnCancelClass, &QPushButton::clicked, uiRoutine, &UIRoutine::onCancelClassClicked);
    connect(ui->btnRescheduleClass, &QPushButton::clicked, uiRoutine, &UIRoutine::onRescheduleClassClicked);

    // Teacher Academic Tools connections
    connect(ui->btnCreateAssessment, &QPushButton::clicked, uiAcademics, &UIAcademics::onCreateAssessmentClicked);
    connect(ui->comboTeacherAssessment, QOverload<int>::of(&QComboBox::currentIndexChanged), uiAcademics, &UIAcademics::onTeacherAssessmentChanged);
    connect(ui->btnSaveGrades, &QPushButton::clicked, uiAcademics, &UIAcademics::onSaveGradesClicked);
    connect(ui->comboAttendanceCourse, QOverload<int>::of(&QComboBox::currentIndexChanged), uiAcademics, &UIAcademics::onAttendanceCourseChanged);
    connect(ui->btnAddClassDate, &QPushButton::clicked, uiAcademics, &UIAcademics::onAddClassDateClicked);
    connect(ui->btnSaveAttendance, &QPushButton::clicked, uiAcademics, &UIAcademics::onSaveAttendanceClicked);

    // Q&A connections
    connect(ui->btnQueryAction, &QPushButton::clicked, uiQueries, &UIQueries::onQueryActionClicked);

    // Admin Panel connections
    connect(ui->tableComboBox, &QComboBox::currentTextChanged, uiAdmin, &UIAdmin::onTableComboBoxChanged);
    connect(ui->btnAddRow, &QPushButton::clicked, uiAdmin, &UIAdmin::onAddRowClicked);
    connect(ui->btnDeleteRow, &QPushButton::clicked, uiAdmin, &UIAdmin::onDeleteRowClicked);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, uiAdmin, &UIAdmin::onSearchTextChanged);
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
        if (ui->tableAttendance->isVisible())
            Utils::adjustColumnWidths(ui->tableAttendance); });
}
