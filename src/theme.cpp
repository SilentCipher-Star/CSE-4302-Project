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

                      "QTabWidget::pane { border: none; border-radius: 12px; background: %2; top: -1px; } "
                      "QTabWidget::tab-bar { alignment: left; } "
                      "QTabBar::tab { background: %1; color: %3; border: none; border-bottom: 3px solid transparent; padding: 10px 20px; margin-right: 2px; font-weight: 600; } "
                      "QTabBar::tab:selected { background-color: %4; color: %1; border-bottom: 3px solid %1; } "
                      "QTabBar::tab:hover:!selected { border: 1px solid %4; border-radius: 4px; } "

                      "QTableWidget, QTableView, QListWidget { border: 1px solid %4; border-radius: 6px; padding: 6px; gridline-color: %4; selection-background-color: %4; selection-color: %1; alternate-background-color: %2; background-color: %1; } "
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
        {"Teal Night", "#0b1f24", "#12343b", "#d9f3f4", "#14b8a6"},
        {"Midnight Blue", "#0f172a", "#1e293b", "#e2e8f0", "#38bdf8"},
        {"Cool Indigo", "#151b2d", "#1f2a44", "#e0e7ff", "#6366f1"},
        {"Charcoal Gold", "#1c1c1c", "#2a2a2a", "#f5f5f5", "#fbbf24"},
        {"Lavender Night", "#1e1b2e", "#2a2640", "#e6e6fa", "#c084fc"},
        {"Soft Ivory", "#f8f5f0", "#e8e2d9", "#2e2a26", "#c97b2a"},
        {"Ocean Mist", "#0c1b2a", "#162d44", "#d6e6f2", "#60a5fa"},
        {"Slate Minimal", "#111827", "#1f2937", "#f3f4f6", "#9ca3af"},
        {"Funky", "#2c1106", "#351c12", "#f3f4f6", "#4f2e0d"},
        {"Paper Grey", "#f3f4f6", "#e5e7eb", "#111827", "#6366f1"},
        {"Mint Cream", "#f0fdf4", "#dcfce7", "#14532d", "#22c55e"},
        {"Graphite", "#181818", "#242424", "#e5e5e5", "#4f9cff"},
        {"Warm Sand", "#faf3e0", "#f4e3c1", "#3a2e1f", "#d97706"},
        {"Deep Plum", "#1a0f1f", "#2a1633", "#f5e6ff", "#d946ef"}};
}