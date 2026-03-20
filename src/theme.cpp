#include "../include/theme.hpp"
#include "../include/csvhandler.hpp"
#include <QStyleFactory>
#include <QPalette>
#include <QDebug>
#include <QPainter>
#include <QPropertyAnimation>
#include <QMouseEvent>
#include <exception>

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
    placeholder.setAlpha(110);
    p.setColor(QPalette::PlaceholderText, placeholder);

    a.setPalette(p);

    // Embed theme colors dynamically through QString mappings
    QString qss = QString(

                      /* Base component defaults */
                      "QWidget  { font-size:%5px; font-family:'%9';"
                      "           color:%3; background-color:%1;"
                      "           selection-background-color:%4; selection-color:%1; }"
                      "QMainWindow { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
                      "              stop:0 %1, stop:0.55 %2, stop:1 %1); }"
                      "QDialog     { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
                      "              stop:0 %1, stop:0.55 %2, stop:1 %1); }"

                      /* Specific application label overrides */
                      "QLabel#label_timerDisplay, QLabel#label_workoutTimerDisplay"
                      "  { font-size:%6px; font-weight:300; color:%4;"
                      "    font-family:'%10',sans-serif; }"
                      "QLabel#label_welcome { font-size:%8px; font-weight:bold; }"

                      /* Interactive click elements */
                      "QPushButton {"
                      "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %4,stop:1 %4);"
                      "  color:%1; border:none; border-radius:18px;"
                      "  padding:8px 18px; font-weight:700; letter-spacing:0.8px; }"
                      "QPushButton:hover {"
                      "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %4,stop:1 %2);"
                      "  border:2px solid %4; color:%3; }"
                      "QPushButton:pressed { padding-top:9px; padding-bottom:7px; }"
                      "QPushButton:disabled {"
                      "  background-color:%2; color:%3; border:1.5px solid %4; border-radius:18px; }"

                      /* Selectable item blocks */
                      "QCheckBox { spacing:10px; }"
                      "QCheckBox::indicator, QListView::indicator {"
                      "  width:22px; height:22px; border:2px solid %4; border-radius:8px; background-color:%2; }"
                      "QCheckBox::indicator:checked, QListView::indicator:checked {"
                      "  background:%4; border-color:%4; }"
                      "QCheckBox::indicator:hover, QListView::indicator:hover { border:2px solid %3; }"

                      /* Single input boxes */
                      "QLineEdit {"
                      "  border:1.5px solid %4; border-radius:14px;"
                      "  padding:9px 14px; background-color:%2; color:%3; }"
                      "QLineEdit:focus {"
                      "  border:2px solid %4;"
                      "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %2,stop:1 %1); }"
                      "QLineEdit:hover { border:1.5px solid %3; }"

                      /* Dropdown elements */
                      "QComboBox {"
                      "  border:1.5px solid %4; border-radius:12px;"
                      "  padding:8px 12px; background-color:%2; color:%3; }"
                      "QComboBox:focus { border:2px solid %4; }"
                      "QComboBox:hover { border:1.5px solid %3; }"
                      "QComboBox::drop-down {"
                      "  subcontrol-origin:padding; subcontrol-position:top right; width:34px;"
                      "  border-left:1px solid %4; border-top-right-radius:13px;"
                      "  border-bottom-right-radius:13px; background:%4; }"
                      "QComboBox::down-arrow {"
                      "  width:10px; height:10px;"
                      "  border-left:5px solid transparent; border-right:5px solid transparent;"
                      "  border-top:6px solid %1; }"
                      "QComboBox QAbstractItemView {"
                      "  background-color:%2; border:1.5px solid %4; border-radius:14px;"
                      "  selection-background-color:%4; selection-color:%1; outline:none; padding:6px; }"
                      "QComboBox QAbstractItemView::item { padding:9px 14px; border-radius:8px; }"
                      "QComboBox QAbstractItemView::item:hover { background-color:%4; color:%1; }"

                      /* Number increment inputs */
                      "QAbstractSpinBox {"
                      "  border:1.5px solid %4; border-radius:12px;"
                      "  padding:8px 12px; background-color:%2; color:%3;"
                      "  selection-background-color:%4; selection-color:%1; }"
                      "QAbstractSpinBox:focus { border:2px solid %4; }"
                      "QAbstractSpinBox::up-button {"
                      "  subcontrol-origin:border; subcontrol-position:top right; width:28px;"
                      "  border-left:1px solid %1; border-bottom:1px solid %1;"
                      "  background:%4; border-top-right-radius:13px; }"
                      "QAbstractSpinBox::up-button:hover { background:%3; }"
                      "QAbstractSpinBox::down-button {"
                      "  subcontrol-origin:border; subcontrol-position:bottom right; width:28px;"
                      "  border-left:1px solid %1; background:%4; border-bottom-right-radius:13px; }"
                      "QAbstractSpinBox::down-button:hover { background:%3; }"

                      /* Framed sections */
                      "QGroupBox {"
                      "  border:1.5px solid %4; border-radius:20px;"
                      "  margin-top:32px; padding-top:24px; font-weight:700;"
                      "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %2,stop:1 %1); }"
                      "QGroupBox::title {"
                      "  subcontrol-origin:margin; subcontrol-position:top left;"
                      "  left:18px; padding:3px 12px; color:%4;"
                      "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %2,stop:1 %1);"
                      "  border-radius:8px; }"

                      /* Status widget items */
                      "QFrame#statsCard {"
                      "  border: 2px solid transparent; }"
                      "QFrame#statsCard:hover {"
                      "  border: 2px solid %2; margin-top: -2px; margin-bottom: 2px; }"

                      /* Major pane selectors */
                      "QTabWidget::pane {"
                      "  border:1.5px solid %4; border-radius:18px;"
                      "  background:%2; top:-1px; padding:14px; margin-top: 10px; }"
                      "QTabWidget::tab-bar { alignment:left; }"
                      "QTabBar::tab {"
                      "  background:%1; color:%3; border:1.5px solid transparent;"
                      "  padding:10px 24px; margin:3px 3px; font-weight:700; border-radius:22px; }"
                      "QTabBar::tab:selected {"
                      "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %4,stop:1 %4);"
                      "  color:%1; border:none; }"
                      "QTabBar::tab:hover:!selected {"
                      "  background:%2; border:1.5px solid %4; }"

                      /* Grid viewers */
                      "QTableWidget, QTableView, QListWidget {"
                      "  border:1.5px solid %4; border-radius:14px; padding:6px;"
                      "  gridline-color:%2; selection-background-color:%4; selection-color:%1;"
                      "  alternate-background-color:%2; background-color:%1; outline:none; }"
                      "QHeaderView::section {"
                      "  background: qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 %2,stop:1 %1);"
                      "  padding:7px 10px; border:none; border-bottom:2px solid %4;"
                      "  border-right:1px solid %4; font-weight:700; text-transform:uppercase;"
                      "  color:%4; font-size:%7px; }"
                      "QHeaderView::section:first { border-top-left-radius:10px; }"
                      "QHeaderView::section:last  { border-right:none; border-top-right-radius:10px; }"
                      "QTableCornerButton::section { background-color:%2; border:none; }"
                      "QTableWidget::item { padding:6px; border-bottom:1px solid %2; }"
                      "QTableWidget::item:hover { background-color:%2; }"
                      "QTableWidget::item:selected {"
                      "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %4,stop:1 %2); color:%1; }"
                      "QTableView::item { padding:6px; border-bottom:1px solid %2; selection-color:%1; }"
                      "QTableView::item:hover { background-color:%4; color:%1; }"
                      "QTableView::item:selected {"
                      "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %4,stop:1 %2); color:%1; }"
                      "QListWidget::item { padding:8px; margin:3px 6px; border-radius:12px; }"
                      "QListWidget::item:selected {"
                      "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %4,stop:1 %2);"
                      "  color:%1; border-radius:12px; }"
                      "QListWidget::item:hover:!selected { background-color:%2; border: 1px solid %4; border-radius:12px; }"

                      /* Layout scrollers */
                      "QScrollBar:vertical   { background:%2; width:10px; margin: 0px; border-radius:5px; }"
                      "QScrollBar:horizontal { background:%2; height:10px; margin: 0px; border-radius:5px; }"
                      "QScrollBar::handle:vertical {"
                      "  background:%4; min-height:30px; border-radius:5px; margin: 2px; }"
                      "QScrollBar::handle:horizontal {"
                      "  background:%4; min-width:30px; border-radius:5px; margin: 2px; }"
                      "QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover"
                      "  { background:%3; }"
                      "QScrollBar::add-line:vertical,   QScrollBar::sub-line:vertical,"
                      "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal"
                      "  { width:0; height:0; background:none; }"
                      "QScrollBar::add-page:vertical,   QScrollBar::sub-page:vertical,"
                      "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal"
                      "  { background:none; }"

                      /* Status trackers */
                      "QProgressBar {"
                      "  border:1.5px solid %4; border-radius:10px; background-color:%2;"
                      "  text-align:center; color:%3; height:24px; }"
                      "QProgressBar::chunk {"
                      "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %4,stop:1 %2);"
                      "  border-radius:9px; }"

                      /* Popout lists */
                      "QMenu { background-color:%2; border:1.5px solid %4; border-radius:14px; padding:6px; }"
                      "QMenu::item { padding:10px 30px 10px 18px; border-radius:8px; color:%3; }"
                      "QMenu::item:selected {"
                      "  background: qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 %4,stop:1 %2); color:%1; }"
                      "QMenu::separator { height:1px; background:%4; margin:5px 12px; }"
                      "QMenu::indicator { width:16px; height:16px; }"

                      /* Multi line inputs */
                      "QTextEdit {"
                      "  border:1.5px solid %4; border-radius:14px;"
                      "  padding:10px; background-color:%2; color:%3; }"
                      "QTextEdit:focus { border:2px solid %4; }"

                      /* Popup hovers */
                      "QToolTip {"
                      "  color:%3; background-color:%2; border:1.5px solid %4;"
                      "  border-radius:10px; padding:8px 12px; }"

                      )
                      .arg(theme.background,       // %1
                           theme.surface,          // %2
                           theme.text,             // %3
                           theme.accent)           // %4
                      .arg(AppFonts::Normal)       // %5
                      .arg(AppFonts::Timer)        // %6
                      .arg(AppFonts::Small)        // %7
                      .arg(AppFonts::Large)        // %8
                      .arg(AppFonts::Family)       // %9
                      .arg(AppFonts::TimerFamily); // %10

    a.setStyleSheet(qss);
}

QVector<AppTheme> ThemeManager::getAvailableThemes()
{
    QVector<AppTheme> themes;
    try
    {
        QVector<QStringList> data = CsvHandler::readCsv("themes.csv");
        for (const auto &row : data)
        {
            if (row.size() >= 5)
                themes.append({row[0], row[1], row[2], row[3], row[4]});
        }
    }
    catch (const std::exception &e)
    {
        qWarning() << "Failed to load themes.csv:" << e.what();
    }
    catch (...)
    {
        qWarning() << "Unknown error loading themes.csv.";
    }

    if (themes.isEmpty())
    {
        themes.append({"Sakura Dream", "#fdf0f3", "#fff5f7", "#3d1a2e", "#e91e8c"});
        themes.append({"Mint Fresh", "#f0fdf4", "#f0fdf4", "#14532d", "#16a34a"});
        themes.append({"Ocean Breeze", "#f0f9ff", "#f0f9ff", "#0c4a6e", "#0ea5e9"});
        themes.append({"Lavender", "#f5f3ff", "#f5f3ff", "#4c1d95", "#8b5cf6"});
        themes.append({"Autumn Warmth", "#fff7ed", "#fff7ed", "#7c2d12", "#f97316"});
    }

    return themes;
}

QVector<AppTheme> ThemeManager::getDarkThemes()
{
    return {
        {"Midnight", "#1a1b2e", "#252641", "#e2e8f0", "#818cf8"},
        {"Obsidian", "#1c1c1e", "#2c2c2e", "#f2f2f7", "#30d158"},
        {"Crimson Night", "#1a0808", "#2d1010", "#fde8e8", "#ef4444"},
        {"Ocean Depth", "#051020", "#0a1e38", "#dff0ff", "#38bdf8"},
        {"Amber Dark", "#1a1200", "#2a1f00", "#fff8e1", "#f59e0b"},
    };
}

ThemeToggle::ThemeToggle(QWidget *parent)
    : QWidget(parent), m_anim(new QPropertyAnimation(this, "offset", this))
{
    setFixedSize(68, 30);
    setCursor(Qt::PointingHandCursor);
    setToolTip("Toggle dark / light mode");
    m_anim->setDuration(180);
    m_anim->setEasingCurve(QEasingCurve::InOutCubic);
}

void ThemeToggle::setDark(bool dark)
{
    m_dark = dark;
    m_offset = dark ? 40.0 : 3.0;
    update();
}

void ThemeToggle::setOffset(qreal v)
{
    m_offset = v;
    update();
}

void ThemeToggle::mousePressEvent(QMouseEvent *)
{
    m_dark = !m_dark;
    m_anim->setStartValue(m_offset);
    m_anim->setEndValue(m_dark ? 40.0 : 3.0);
    m_anim->start();
    emit toggled(m_dark);
}

void ThemeToggle::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Generate base indicator capsule
    QColor pill = m_dark ? QColor(67, 56, 202) : QColor(251, 146, 60);
    p.setBrush(pill);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect(), 15, 15);

    // Overlay animated toggle dot
    p.setBrush(Qt::white);
    p.drawEllipse(QRectF(m_offset, 3, 24, 24));

    // Place unicode icon relative to toggle position
    p.setPen(m_dark ? QColor(67, 56, 202) : QColor(234, 88, 12));
    QFont f(AppFonts::Family);
    f.setPixelSize(AppFonts::Small);
    p.setFont(f);
    p.drawText(QRectF(m_offset, 3, 24, 24), Qt::AlignCenter,
               m_dark ? QString("\u263D") : QString("\u2600"));
}