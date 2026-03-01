#pragma once
#include <QObject>
#include <QTimer>

class Timer : public QObject
{
    Q_OBJECT
public:
    explicit Timer(QObject *parent = nullptr);

    void start(int minutes);
    void startStopwatch(int targetMinutes = 0);
    void pause();
    void stop();
    double getElapsedMinutes() const;

signals:
    void timeUpdated(QString time, float progress);
    void finished();

private slots:
    void onTimeout();

private:
    QTimer *internalTimer;
    int remainingTime = 0;
    int totalTime = 0;
    int elapsedTime = 0;
    bool isPaused;
    bool isStopwatch;
};