#pragma once

#include <QMainWindow>
#include <QTableWidgetItem>
#include <QListWidgetItem>
#include <QPushButton>
#include "appmanager.hpp"
#include "theme.hpp"
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "theme_toggle.hpp"

// Forward declarations
QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

// Forward declare UI modules
class UIDashboard;
class UIPlanner;
class UIHabits;
class UITimers;
class UIRoutine;
class UIAcademics;
class UIQueries;
class UIAdmin;
class UITamagotchi;
class UICalendar;
class UILostFound;

// The main application window
class MainWindow : public QMainWindow, public IDataObserver
{
    Q_OBJECT

public:
    MainWindow(QString role, int userId, QString userName, QWidget *parent = nullptr);
    ~MainWindow();

    // Observer Interface
    void onDataChanged(DataType type) override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MainWindow *ui;
    AcadenceManager myManager;

    // User Session Data
    QString userRole;
    int userId;
    QString userName;

    // UI Modules
    UIDashboard *uiDashboard;
    UIPlanner *uiPlanner;
    UIHabits *uiHabits;
    UITimers *uiTimers;
    UIRoutine *uiRoutine;
    UIAcademics  *uiAcademics;
    UIQueries    *uiQueries;
    UIAdmin      *uiAdmin;
    UITamagotchi *uiTamagotchi = nullptr;
    UICalendar   *uiCalendar   = nullptr;
    UILostFound  *uiLostFound  = nullptr;

    // Initialization Helpers
    void setupTables();
    void setupConnections();
    void toggleDarkMode(bool isDark);
    void onThemeClicked();

    // Undo/Redo buttons
    QPushButton *m_undoBtn    = nullptr;
    QPushButton *m_redoBtn    = nullptr;
    void updateUndoRedoButtons();

    ThemeToggle *m_toggle     = nullptr;
    QPushButton *m_themeBtn   = nullptr;
    bool         m_darkMode   = false;
    int          m_darkThemeIndex = 0;
    AppTheme     m_userTheme;
};
