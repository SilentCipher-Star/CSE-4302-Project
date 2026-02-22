#include "../include/theme.hpp"
#include <QStyleFactory>
#include <QPalette>

void ThemeManager::applyTheme(QApplication &a, const AppTheme &theme)
{
    a.setStyle(QStyleFactory::create("Fusion"));

    QColor cBg(theme.background);
    QColor cSurf(theme.surface);
    QColor cText(theme.text);
    QColor cAcc(theme.accent);

    QPalette p;
    p.setColor(QPalette::Window, cBg);
    p.setColor(QPalette::WindowText, cText);
    p.setColor(QPalette::Base, cSurf);
    p.setColor(QPalette::AlternateBase, cBg);
    p.setColor(QPalette::ToolTipBase, cSurf);
    p.setColor(QPalette::ToolTipText, cText);
    p.setColor(QPalette::Text, cText);
    p.setColor(QPalette::Button, cAcc);
    p.setColor(QPalette::ButtonText, cBg);
    p.setColor(QPalette::Link, cAcc);
    p.setColor(QPalette::Highlight, cAcc);
    p.setColor(QPalette::HighlightedText, cBg);

    QColor placeholder = cText;
    placeholder.setAlpha(128);
    p.setColor(QPalette::PlaceholderText, placeholder);

    a.setPalette(p);

    QString qss = QString(
                      "QWidget { font-size: 18px; font-family: 'Product Sans', 'Segoe UI', sans-serif; color: %3; background-color: %1; selection-background-color: %4; selection-color: %1; }"
                      "QMainWindow { background-color: %1; }"

                      "QLabel#label_timerDisplay, QLabel#label_workoutTimerDisplay { font-size: 80px; font-weight: 300; color: %4; font-family: 'Product Sans', sans-serif; }"

                      "QPushButton { background-color: %4; color: %1; border: none; border-radius: 6px; padding: 8px 16px; font-weight: 600; letter-spacing: 0.5px; } "
                      "QPushButton:hover { border: 2px solid %3; } "
                      "QPushButton:pressed { padding-top: 9px; padding-bottom: 7px; } "
                      "QPushButton:disabled { background-color: %2; color: %3; border: 1px solid %3; }"

                      "QCheckBox { spacing: 8px; } "

                      "QLineEdit, QComboBox { border: 1px solid %4; border-radius: 6px; padding: 8px; background-color: %2; color: %3; } "
                      "QLineEdit:focus, QComboBox:focus { border: 2px solid %4; background-color: %2; }"

                      "QAbstractSpinBox { border: 1px solid %4; border-radius: 6px; padding: 8px; background-color: %2; color: %3; selection-background-color: %4; selection-color: %1; }"
                      "QAbstractSpinBox:focus { border: 2px solid %4; }"
                      "QAbstractSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 24px; border-left: 1px solid %1; border-bottom: 1px solid %1; background-color: %4; border-top-right-radius: 5px; }"
                      "QAbstractSpinBox::up-button:hover { background-color: %3; }"
                      "QAbstractSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 24px; border-left: 1px solid %1; border-top: 0px; background-color: %4; border-bottom-right-radius: 5px; }"
                      "QAbstractSpinBox::down-button:hover { background-color: %3; }"

                      "QGroupBox { border: 1px solid %4; border-radius: 8px; margin-top: 24px; padding-top: 16px; font-weight: 600; background-color: transparent; } "
                      "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 12px; padding: 0 6px; color: %4; } "

                      "QTabWidget::pane { border: 1px solid %4; border-radius: 8px; background: %2; top: -1px; } "
                      "QTabWidget::tab-bar { alignment: left; } "
                      "QTabBar::tab { background: %1; color: %3; border: none; border-bottom: 3px solid transparent; padding: 10px 20px; margin-right: 2px; font-weight: 600; } "
                      "QTabBar::tab:selected { background-color: %4; color: %1; border-bottom: 3px solid %1; } "
                      "QTabBar::tab:hover:!selected { border: 1px solid %4; border-radius: 4px; } "

                      "QTableWidget, QTableView, QListWidget { border: 1px solid %4; border-radius: 6px; gridline-color: %4; selection-background-color: %4; selection-color: %1; alternate-background-color: %1; background-color: %2; } "
                      "QHeaderView::section { background-color: %1; padding: 8px; border: none; border-bottom: 2px solid %4; font-weight: 700; text-transform: uppercase; color: %3; font-size: 12px; } "
                      "QTableCornerButton::section { background-color: %1; border: none; } "
                      "QTableWidget::item { padding: 6px; border-bottom: 1px solid %4; } "
                      "QToolTip { color: %3; background-color: %2; border: 1px solid %4; }")
                      .arg(theme.background,
                           theme.surface,
                           theme.text,
                           theme.accent);

    a.setStyleSheet(qss);
}

QVector<AppTheme> ThemeManager::getAvailableThemes()
{
    return {
        {"Cyberpunk", "#000b1e", "#0a192f", "#34edf3", "#f715ab"},
        {"Nord", "#2E3440", "#3B4252", "#D8DEE9", "#88C0D0"},
        {"Gruvbox", "#282828", "#3c3836", "#ebdbb2", "#fe8019"},
        {"Solarized", "#002b36", "#073642", "#839496", "#b58900"},
        {"Monokai", "#272822", "#3e3d32", "#f8f8f2", "#a6e22e"}};
}