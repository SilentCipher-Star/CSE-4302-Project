#include "../include/theme.hpp"
#include "../include/csvhandler.hpp"
#include <QStyleFactory>
#include <QPalette>

const QString AppFonts::Normal = "20px";
const QString AppFonts::Small = "18px";
const QString AppFonts::Large = "36px";
const QString AppFonts::Timer = "100px";
const QString AppFonts::Title = "54pt";

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
                      "QWidget { font-size: %5; font-family: 'Product Sans', 'Segoe UI', sans-serif; color: %3; background-color: %1; selection-background-color: %4; selection-color: %1; }"
                      "QMainWindow { background-color: %1; }"

                      "QLabel#label_timerDisplay, QLabel#label_workoutTimerDisplay { font-size: %6; font-weight: 300; color: %4; font-family: 'Product Sans', sans-serif; }"
                      "QLabel#label_welcome { font-size: %8; font-weight: bold; }"

                      "QPushButton { background-color: %4; color: %1; border: none; border-radius: 6px; padding: 8px 16px; font-weight: 600; letter-spacing: 0.5px; } "
                      "QPushButton:hover { border: 2px solid %3; } "
                      "QPushButton:pressed { padding-top: 9px; padding-bottom: 7px; } "
                      "QPushButton:disabled { background-color: %2; color: %3; border: 1px solid %3; }"

                      "QCheckBox { spacing: 10px; } "
                      "QCheckBox::indicator, QListView::indicator { width: 24px; height: 24px; border: 2px solid %4; border-radius: 6px; background-color: %2; } "
                      "QCheckBox::indicator:checked, QListView::indicator:checked { background-color: %4; border-color: %4; } "
                      "QCheckBox::indicator:hover, QListView::indicator:hover { border-color: %3; } "

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
                      "QHeaderView::section { background-color: %1; padding: 8px; border: none; border-bottom: 2px solid %4; font-weight: 700; text-transform: uppercase; color: %3; font-size: %7; } "
                      "QTableCornerButton::section { background-color: %1; border: none; } "
                      "QTableWidget::item { padding: 6px; border-bottom: 1px solid %4; } "
                      "QListWidget::item { padding: 6px; margin: 4px; border-bottom: 1px solid %4; } "
                      "QListWidget::item:selected { background-color: %4; color: %1; border-radius: 6px; border: none; } "
                      "QListWidget::item:hover:!selected { background-color: %2; border-radius: 6px; } "
                      "QToolTip { color: %3; background-color: %2; border: 1px solid %4; }")
                      .arg(theme.background,
                           theme.surface,
                           theme.text,
                           theme.accent,
                           AppFonts::Normal,
                           AppFonts::Timer,
                           AppFonts::Small,
                           AppFonts::Large);

    a.setStyleSheet(qss);
}

QVector<AppTheme> ThemeManager::getAvailableThemes()
{
    QVector<AppTheme> themes;
    try
    {
        QVector<QStringList> data = CsvHandler::readCsv("../data/themes.csv");
        for (const auto &row : data)
        {
            if (row.size() >= 5)
            {
                themes.append({row[0], row[1], row[2], row[3], row[4]});
            }
        }
    }
    catch (...)
    {
    }

    if (themes.isEmpty())
        themes.append({"Teal Night", "#0b1f24", "#12343b", "#d9f3f4", "#14b8a6"});

    return themes;
}