#pragma once

#include <QObject>
#include <QWidget>
#include <QCalendarWidget>
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDate>
#include <QMap>
#include <QVector>
#include <QString>
#include <QTextCharFormat>
#include "../include/appmanager.hpp"

struct CalendarEvent {
    QString title;
    QString type;   // "exam" or "class"
    QString detail;
    QString color;
};

class UICalendar : public QObject
{
    Q_OBJECT

public:
    explicit UICalendar(AcadenceManager *manager, const QString &role,
                        int userId, QObject *parent = nullptr);

    QWidget *getWidget() const { return m_container; }
    void refresh();

private slots:
    void onDateSelected(QDate date);
    void onPageChanged(int year, int month);

private:
    AcadenceManager *m_mgr;
    QString          m_role;
    int              m_userId;
    int              m_semester = -1;

    QWidget         *m_container;
    QCalendarWidget *m_calendar;
    QLabel          *m_dayTitle;
    QVBoxLayout     *m_eventLayout;
    QWidget         *m_eventContainer;

    QMap<QDate, QVector<CalendarEvent>> m_events;

    void buildWidget();
    void loadEvents();
    void showDayEvents(QDate date);
};
