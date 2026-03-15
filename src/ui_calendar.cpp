#include "../include/ui_calendar.hpp"
#include "../include/theme.hpp"
#include "../include/student.hpp"
#include <QScrollArea>
#include <QFrame>
#include <QFont>

static QString dayNameFromQt(int qtDow)
{
    // Qt dayOfWeek: 1=Monday ... 6=Saturday, 7=Sunday
    static const QString names[] = {
        "", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    return names[qtDow];
}

UICalendar::UICalendar(AcadenceManager *manager, const QString &role,
                       int userId, QObject *parent)
    : QObject(parent), m_mgr(manager), m_role(role), m_userId(userId)
{
    if (role == "student")
    {
        Student *stu = m_mgr->getStudent(userId);
        if (stu)
        {
            m_semester = stu->getSemester();
            delete stu;
        }
    }
    buildWidget();
    loadEvents();
    showDayEvents(QDate::currentDate());
}

void UICalendar::buildWidget()
{
    m_container = new QWidget();
    QHBoxLayout *root = new QHBoxLayout(m_container);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(16);

    // ── Calendar widget ──
    m_calendar = new QCalendarWidget();
    m_calendar->setGridVisible(true);
    m_calendar->setMinimumWidth(480);
    m_calendar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_calendar->setSelectedDate(QDate::currentDate());
    root->addWidget(m_calendar, 3);

    // ── Right panel ──
    QWidget *panel = new QWidget();
    panel->setFixedWidth(290);
    QVBoxLayout *pv = new QVBoxLayout(panel);
    pv->setSpacing(8);
    pv->setContentsMargins(0, 0, 0, 0);

    m_dayTitle = new QLabel("Select a date");
    m_dayTitle->setStyleSheet(QString("font-size:%1px; font-weight:bold; padding:6px;"
                                      "border-bottom: 2px solid palette(mid);")
                                  .arg(AppFonts::Normal));
    m_dayTitle->setAlignment(Qt::AlignCenter);
    pv->addWidget(m_dayTitle);

    // Legend
    QHBoxLayout *legend = new QHBoxLayout();
    auto addLegend = [&](const QString &color, const QString &label)
    {
        QLabel *dot = new QLabel("●");
        dot->setStyleSheet(QString("color:%1; font-size:%2px;").arg(color).arg(AppFonts::Normal));
        dot->setFixedWidth(18);
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet(QString("font-size:%1px;").arg(AppFonts::Small));
        legend->addWidget(dot);
        legend->addWidget(lbl);
        legend->addSpacing(8);
    };
    addLegend("#e74c3c", "Exam");
    addLegend("#3498db", "Class");
    addLegend("#8e44ad", "Both");
    legend->addStretch();
    pv->addLayout(legend);

    // Scrollable event list
    QScrollArea *scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    m_eventContainer = new QWidget();
    m_eventLayout = new QVBoxLayout(m_eventContainer);
    m_eventLayout->setSpacing(6);
    m_eventLayout->setContentsMargins(4, 4, 4, 4);
    m_eventLayout->addStretch();

    scroll->setWidget(m_eventContainer);
    pv->addWidget(scroll, 1);

    root->addWidget(panel, 0);

    connect(m_calendar, &QCalendarWidget::clicked,
            this, &UICalendar::onDateSelected);
    connect(m_calendar, &QCalendarWidget::currentPageChanged,
            this, &UICalendar::onPageChanged);
}

void UICalendar::loadEvents()
{
    m_events.clear();
    m_calendar->setDateTextFormat(QDate(), QTextCharFormat()); // reset all

    QTextCharFormat examFmt;
    examFmt.setBackground(QColor(231, 76, 60, 110));
    examFmt.setFontWeight(QFont::Bold);

    QTextCharFormat classFmt;
    classFmt.setBackground(QColor(52, 152, 219, 80));

    QTextCharFormat bothFmt;
    bothFmt.setBackground(QColor(142, 68, 173, 110));
    bothFmt.setFontWeight(QFont::Bold);

    // ── Assessments ──
    QVector<Assessment> assessments = m_mgr->getStudentAssessments(m_userId);
    for (const auto &a : assessments)
    {
        QDate d = QDate::fromString(a.getDate(), "yyyy-MM-dd");
        if (!d.isValid())
            continue;
        CalendarEvent ev;
        ev.title = a.getTitle();
        ev.type = "exam";
        ev.detail = a.getCourseName() + "  |  Max: " + QString::number(a.getMaxMarks()) + " marks";
        ev.color = "#e74c3c";
        m_events[d].append(ev);
    }

    // ── Routine – mark weekdays across the shown month ──
    int year = m_calendar->yearShown();
    int month = m_calendar->monthShown();
    QDate first(year, month, 1);
    int daysInMonth = first.daysInMonth();

    for (int d = 1; d <= daysInMonth; ++d)
    {
        QDate date(year, month, d);
        QString dName = dayNameFromQt(date.dayOfWeek());
        QVector<RoutineSession> sessions = m_mgr->getRoutineForDay(dName, m_semester);
        for (const auto &s : sessions)
        {
            CalendarEvent ev;
            ev.title = s.getCourseCode() + "  " + s.getCourseName();
            ev.type = "class";
            ev.detail = s.getStartTime() + " – " + s.getEndTime() + "  |  Room: " + s.getRoom();
            ev.color = "#3498db";
            m_events[date].append(ev);
        }
    }

    // ── Apply formats ──
    for (auto it = m_events.cbegin(); it != m_events.cend(); ++it)
    {
        bool hasExam = false, hasClass = false;
        for (const auto &ev : it.value())
        {
            if (ev.type == "exam")
                hasExam = true;
            if (ev.type == "class")
                hasClass = true;
        }
        if (hasExam && hasClass)
            m_calendar->setDateTextFormat(it.key(), bothFmt);
        else if (hasExam)
            m_calendar->setDateTextFormat(it.key(), examFmt);
        else if (hasClass)
            m_calendar->setDateTextFormat(it.key(), classFmt);
    }

    // Underline today
    QDate today = QDate::currentDate();
    QTextCharFormat todayFmt = m_calendar->dateTextFormat(today);
    todayFmt.setFontUnderline(true);
    m_calendar->setDateTextFormat(today, todayFmt);
}

void UICalendar::showDayEvents(QDate date)
{
    m_dayTitle->setText(date.toString("dddd, d MMMM yyyy"));

    // Clear previous cards
    QLayoutItem *item;
    while ((item = m_eventLayout->takeAt(0)) != nullptr)
    {
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    const QVector<CalendarEvent> &events = m_events.value(date);
    if (events.isEmpty())
    {
        QLabel *empty = new QLabel("No events on this day.");
        empty->setStyleSheet(QString("color:palette(text); font-size:%1px; padding:16px;").arg(AppFonts::Small));
        empty->setAlignment(Qt::AlignCenter);
        m_eventLayout->addWidget(empty);
    }
    else
    {
        for (const auto &ev : events)
        {
            QFrame *card = new QFrame();
            card->setFrameShape(QFrame::StyledPanel);
            card->setStyleSheet(
                QString("QFrame { border-left: 4px solid %1; background: palette(base);"
                        "border-radius: 4px; }")
                    .arg(ev.color));

            QVBoxLayout *cl = new QVBoxLayout(card);
            cl->setContentsMargins(10, 7, 10, 7);
            cl->setSpacing(3);

            QLabel *badge = new QLabel(ev.type == "exam" ? "EXAM" : "CLASS");
            badge->setStyleSheet(
                QString("font-size:%2px; font-weight:bold; color:white; background:%1;"
                        "border-radius:3px; padding:1px 5px; border:none;")
                    .arg(ev.color)
                    .arg(AppFonts::Tiny));
            badge->setFixedHeight(18);
            badge->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

            QLabel *title = new QLabel(ev.title);
            title->setStyleSheet(QString("font-weight:bold; font-size:%1px; color:palette(text); border:none; background:transparent;").arg(AppFonts::Small));
            title->setWordWrap(true);

            QLabel *detail = new QLabel(ev.detail);
            detail->setStyleSheet(QString("font-size:%1px; color:palette(text); border:none; background:transparent;").arg(AppFonts::Small));
            detail->setWordWrap(true);

            cl->addWidget(badge);
            cl->addWidget(title);
            cl->addWidget(detail);
            m_eventLayout->addWidget(card);
        }
    }
    m_eventLayout->addStretch();
}

void UICalendar::onDateSelected(QDate date)
{
    showDayEvents(date);
}

void UICalendar::onPageChanged(int year, int month)
{
    Q_UNUSED(year);
    Q_UNUSED(month);
    loadEvents();
    showDayEvents(m_calendar->selectedDate());
}

void UICalendar::refresh()
{
    loadEvents();
    showDayEvents(m_calendar->selectedDate());
}
