#pragma once

#include <QObject>
#include <QWidget>
#include <QTimer>
#include <QElapsedTimer>
#include <QPixmap>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QFrame>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "../include/appmanager.hpp"

// ─── Character state ─────────────────────────────────────────────────────────
enum class TamaState
{
    Happy,
    Neutral,
    Tired,
    Stressed,
    Sick
};

// ─── Computed stats (all 0-100) ───────────────────────────────────────────────
struct TamaStats
{
    int health = 50; // from habit streaks
    int mood = 50;   // from daily prayer completion
    int energy = 60; // managed via buttons + decay
    int brain = 50;  // from CGPA
    bool examSoon = false;
    int daysToExam = 999;
    QString examName;
};

// ─── Canvas: handles all drawing + animation ──────────────────────────────────
class TamagotchiCanvas : public QWidget
{
    Q_OBJECT
public:
    explicit TamagotchiCanvas(QWidget *parent = nullptr);

    void setState(TamaState state);
    void setStats(const TamaStats &stats);
    void setAccentColor(const QString &accent);

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onAnimTick();

private:
    TamaState m_state = TamaState::Neutral;
    TamaStats m_stats;
    QTimer *m_animTimer;
    int m_frame = 0;
    QElapsedTimer m_elapsedTimer;
    QPixmap m_cachedRoom;
    bool m_cacheValid = false;
    QString m_accent = "#e8a0b0";

    void drawRoom(QPainter &p);
    void drawCharacter(QPainter &p, qreal cx, qreal cy, qreal sc);
    void drawHappyFace(QPainter &p, qreal cx, qreal cy, qreal r);
    void drawNeutralFace(QPainter &p, qreal cx, qreal cy, qreal r);
    void drawTiredFace(QPainter &p, qreal cx, qreal cy, qreal r);
    void drawStressedFace(QPainter &p, qreal cx, qreal cy, qreal r);
    void drawSickFace(QPainter &p, qreal cx, qreal cy, qreal r);
    void drawAccessories(QPainter &p, qreal cx, qreal cy, qreal r);
    void drawEffects(QPainter &p, qreal cx, qreal cy, qreal sc);
};

// ─── UITamagotchi: module wrapper (same pattern as UICalendar) ────────────────
class UITamagotchi : public QObject
{
    Q_OBJECT
public:
    explicit UITamagotchi(AcadenceManager *manager, const QString &role,
                          int userId, QObject *parent = nullptr);

    QWidget *getWidget() const { return m_container; }
    void refreshTamagotchi();

private slots:
    void onFeedClicked();
    void onStudyClicked();
    void onSleepClicked();
    void onStatDecayTick();

private:
    AcadenceManager *m_mgr;
    QString m_role;
    int m_userId;
    int m_energy = 60;

    QWidget *m_container;
    TamagotchiCanvas *m_canvas;
    QProgressBar *m_pbHealth;
    QProgressBar *m_pbMood;
    QProgressBar *m_pbEnergy;
    QProgressBar *m_pbBrain;
    QLabel *m_lblState;
    QLabel *m_lblMessage;
    QPushButton *m_btnFeed;
    QPushButton *m_btnStudy;
    QPushButton *m_btnSleep;
    QTimer *m_decayTimer;

    TamaStats m_stats;

    void buildWidget();
    TamaStats computeStats();
    TamaState stateFromStats(const TamaStats &s);
    QString messageForState(TamaState state, const TamaStats &s);
    void applyStats();
    QString pbStyle(const QString &color);
};
