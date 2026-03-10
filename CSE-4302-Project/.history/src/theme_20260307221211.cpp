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
                      /* ── Base ─────────────────────────────────────────────────────────── */
                      "QWidget { font-size: %5; font-family: 'Product Sans', 'Segoe UI', sans-serif; color: %3; background-color: %1; selection-background-color: %4; selection-color: %1; }"
                      "QMainWindow { background-color: %1; }"
                      "QDialog { background-color: %1; }"

                      /* ── Special Labels ─────────────────────────────────────────────── */
                      "QLabel#label_timerDisplay, QLabel#label_workoutTimerDisplay { font-size: %6; font-weight: 300; color: %4; font-family: 'Product Sans', sans-serif; }"
                      "QLabel#label_welcome { font-size: %8; font-weight: bold; }"

                      /* ── Buttons ────────────────────────────────────────────────────── */
                      "QPushButton { background-color: %4; color: %1; border: none; border-radius: 10px; padding: 10px 22px; font-weight: 700; letter-spacing: 0.6px; }"
                      "QPushButton:hover { border: 2px solid %3; }"
                      "QPushButton:pressed { padding-top: 11px; padding-bottom: 9px; }"
                      "QPushButton:disabled { background-color: %2; color: %3; border: 1px solid %4; border-radius: 10px; }"

                      /* ── Checkboxes ─────────────────────────────────────────────────── */
                      "QCheckBox { spacing: 10px; }"
                      "QCheckBox::indicator, QListView::indicator { width: 24px; height: 24px; border: 2px solid %4; border-radius: 6px; background-color: %2; }"
                      "QCheckBox::indicator:checked, QListView::indicator:checked { background-color: %4; border-color: %4; }"
                      "QCheckBox::indicator:hover, QListView::indicator:hover { border: 2px solid %3; }"

                      /* ── Line Edits ─────────────────────────────────────────────────── */
                      "QLineEdit { border: 1px solid %4; border-radius: 8px; padding: 9px 12px; background-color: %2; color: %3; }"
                      "QLineEdit:focus { border: 2px solid %4; }"
                      "QLineEdit:hover { border: 1px solid %3; }"

                      /* ── ComboBox ───────────────────────────────────────────────────── */
                      "QComboBox { border: 1px solid %4; border-radius: 8px; padding: 9px 12px; background-color: %2; color: %3; }"
                      "QComboBox:focus { border: 2px solid %4; }"
                      "QComboBox:hover { border: 1px solid %3; }"
                      "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; width: 32px; border-left: 1px solid %4; border-top-right-radius: 7px; border-bottom-right-radius: 7px; background-color: %4; }"
                      "QComboBox::down-arrow { width: 10px; height: 10px; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 6px solid %1; }"
                      "QComboBox QAbstractItemView { background-color: %2; border: 1px solid %4; border-radius: 8px; selection-background-color: %4; selection-color: %1; outline: none; padding: 4px; }"
                      "QComboBox QAbstractItemView::item { padding: 8px 12px; border-radius: 4px; }"
                      "QComboBox QAbstractItemView::item:hover { background-color: %4; color: %1; }"

                      /* ── Spin Boxes ─────────────────────────────────────────────────── */
                      "QAbstractSpinBox { border: 1px solid %4; border-radius: 8px; padding: 9px 12px; background-color: %2; color: %3; selection-background-color: %4; selection-color: %1; }"
                      "QAbstractSpinBox:focus { border: 2px solid %4; }"
                      "QAbstractSpinBox::up-button { subcontrol-origin: border; subcontrol-position: top right; width: 26px; border-left: 1px solid %1; border-bottom: 1px solid %1; background-color: %4; border-top-right-radius: 7px; }"
                      "QAbstractSpinBox::up-button:hover { background-color: %3; }"
                      "QAbstractSpinBox::down-button { subcontrol-origin: border; subcontrol-position: bottom right; width: 26px; border-left: 1px solid %1; border-top: none; background-color: %4; border-bottom-right-radius: 7px; }"
                      "QAbstractSpinBox::down-button:hover { background-color: %3; }"

                      /* ── Group Boxes ────────────────────────────────────────────────── */
                      "QGroupBox { border: 1px solid %4; border-radius: 10px; margin-top: 26px; padding-top: 18px; font-weight: 700; background-color: %2; }"
                      "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left; left: 14px; padding: 2px 8px; color: %4; background-color: %2; border-radius: 4px; }"

                      /* ── Tab Widget ─────────────────────────────────────────────────── */
                      "QTabWidget::pane { border: 1px solid %4; border-radius: 12px; background: %2; top: -1px; }"
                      "QTabWidget::tab-bar { alignment: left; }"
                      "QTabBar::tab { background: %1; color: %3; border: none; border-bottom: 3px solid transparent; padding: 11px 22px; margin-right: 2px; font-weight: 700; border-top-left-radius: 6px; border-top-right-radius: 6px; }"
                      "QTabBar::tab:selected { background-color: %4; color: %1; border-bottom: 3px solid %1; }"
                      "QTabBar::tab:hover:!selected { background-color: %2; border-bottom: 3px solid %4; }"

                      /* ── Tables & Lists ─────────────────────────────────────────────── */
                      "QTableWidget, QTableView, QListWidget { border: 1px solid %4; border-radius: 8px; padding: 4px; gridline-color: %2; selection-background-color: %4; selection-color: %1; alternate-background-color: %2; background-color: %1; outline: none; }"
                      "QHeaderView::section { background-color: %2; padding: 10px 8px; border: none; border-bottom: 2px solid %4; border-right: 1px solid %4; font-weight: 700; text-transform: uppercase; color: %4; font-size: %7; }"
                      "QHeaderView::section:first { border-top-left-radius: 6px; }"
                      "QHeaderView::section:last { border-right: none; border-top-right-radius: 6px; }"
                      "QTableCornerButton::section { background-color: %2; border: none; }"
                      "QTableWidget::item { padding: 8px; border-bottom: 1px solid %2; }"
                      "QTableWidget::item:hover { background-color: %2; }"
                      "QTableView::item { padding: 8px; border-bottom: 1px solid %2; }"
                      "QTableView::item:hover { background-color: %2; }"
                      "QListWidget::item { padding: 8px; margin: 2px 4px; border-radius: 6px; }"
                      "QListWidget::item:selected { background-color: %4; color: %1; border-radius: 6px; }"
                      "QListWidget::item:hover:!selected { background-color: %2; border-radius: 6px; }"

                      /* ── Scroll Bars ─────────────────────────────────────────────────── */
                      "QScrollBar:vertical { background: %1; width: 10px; margin: 0; border-radius: 5px; }"
                      "QScrollBar::handle:vertical { background: %4; border-radius: 5px; min-height: 32px; }"
                      "QScrollBar::handle:vertical:hover { background: %3; }"
                      "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; background: none; }"
                      "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"
                      "QScrollBar:horizontal { background: %1; height: 10px; margin: 0; border-radius: 5px; }"
                      "QScrollBar::handle:horizontal { background: %4; border-radius: 5px; min-width: 32px; }"
                      "QScrollBar::handle:horizontal:hover { background: %3; }"
                      "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; background: none; }"
                      "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; }"

                      /* ── Progress Bar ────────────────────────────────────────────────── */
                      "QProgressBar { border: 1px solid %4; border-radius: 8px; background-color: %2; text-align: center; color: %3; height: 22px; }"
                      "QProgressBar::chunk { background-color: %4; border-radius: 7px; }"

                      /* ── Menus ───────────────────────────────────────────────────────── */
                      "QMenu { background-color: %2; border: 1px solid %4; border-radius: 8px; padding: 4px; }"
                      "QMenu::item { padding: 9px 28px 9px 16px; border-radius: 5px; color: %3; }"
                      "QMenu::item:selected { background-color: %4; color: %1; }"
                      "QMenu::separator { height: 1px; background: %4; margin: 4px 10px; }"
                      "QMenu::indicator { width: 16px; height: 16px; }"

                      /* ── Text Edit ───────────────────────────────────────────────────── */
                      "QTextEdit { border: 1px solid %4; border-radius: 8px; padding: 8px; background-color: %2; color: %3; }"
                      "QTextEdit:focus { border: 2px solid %4; }"

                      /* ── Tooltips ────────────────────────────────────────────────────── */
                      "QToolTip { color: %3; background-color: %2; border: 1px solid %4; border-radius: 6px; padding: 6px 10px; }")
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