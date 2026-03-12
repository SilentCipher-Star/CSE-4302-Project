#pragma once
#include <QObject>
#include <QTimer>

class QSoundEffect;

class Timer : public QObject
{
    Q_OBJECT
public:
    explicit Timer(QObject *parent = nullptr);

    void start(double minutes);
    void startStopwatch(double targetMinutes = 0);
    void pause();
    void stop();
    double getElapsedMinutes() const;

signals:
    void timeUpdated(QString time, float progress);
    void finished();

private slots:
    void onTimeout();

private:
    void emitCountdownTime(int milliseconds);
    void emitStopwatchTime(int milliseconds);
    QTimer *internalTimer;
    int remainingTime = 0;
    int totalTime = 0;
    int elapsedTime = 0;
    int lastSecond = -1;
    bool isPaused;
    bool isStopwatch;
    QSoundEffect *tickSound = nullptr;
    QSoundEffect *endSound = nullptr;
};