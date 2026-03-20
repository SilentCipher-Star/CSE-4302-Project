#include "../include/timer.hpp"
#include "../include/assetmanager.hpp"
#include <QSoundEffect>
#include <QCoreApplication>
#include <QUrl>
#include <QDebug>
#include <exception>

// Define constants at module level
constexpr int TIMER_INTERVAL_MS = 50;
constexpr float DEFAULT_VOLUME = 0.5f;
constexpr int MS_PER_SECOND = 1000;
constexpr int MS_PER_MINUTE = 60000;

Timer::Timer(QObject *parent)
    : QObject(parent), remainingTime(0), totalTime(0), elapsedTime(0),
      lastSecond(-1), isPaused(false), isStopwatch(false)
{
    internalTimer = new QTimer(this);
    connect(internalTimer, &QTimer::timeout, this, &Timer::onTimeout);
    tickSound = AssetManager::getSound("tick");
    endSound = AssetManager::getSound("end");
}

void Timer::start(double minutes)
{
    if (minutes <= 0)
        return;

    remainingTime = static_cast<int>(minutes * MS_PER_MINUTE);
    totalTime = remainingTime;
    isPaused = false;
    isStopwatch = false;
    internalTimer->start(TIMER_INTERVAL_MS);
    lastSecond = remainingTime / MS_PER_SECOND;
    onTimeout();
}

void Timer::startStopwatch(double targetMinutes)
{
    if (targetMinutes < 0)
        return;

    isStopwatch = true;
    elapsedTime = 0;
    totalTime = static_cast<int>(targetMinutes * MS_PER_MINUTE);
    isPaused = false;
    internalTimer->start(TIMER_INTERVAL_MS);
    lastSecond = 0;
    onTimeout();
}

void Timer::pause()
{
    if (internalTimer->isActive())
    {
        internalTimer->stop();
        if (tickSound && tickSound->isPlaying())
            tickSound->stop();
        isPaused = true;
    }
    else if (isPaused)
    {
        bool canResume = false;
        if (isStopwatch)
        {
            if (totalTime <= 0 || elapsedTime < totalTime)
                canResume = true;
        }
        else if (remainingTime > 0)
            canResume = true;

        if (canResume)
        {
            internalTimer->start(TIMER_INTERVAL_MS);
            isPaused = false;
        }
    }
}

void Timer::stop()
{
    internalTimer->stop();
    if (tickSound && tickSound->isPlaying())
        tickSound->stop();
    isPaused = false;
    remainingTime = 0;
    elapsedTime = 0;
    lastSecond = -1;
    emit timeUpdated("00:00", 0.0f);
}

double Timer::getElapsedMinutes() const
{
    if (isStopwatch)
    {
        return (double)elapsedTime / MS_PER_MINUTE;
    }
    else
    {
        if (totalTime <= 0)
            return 0.0;
        return (double)(totalTime - remainingTime) / MS_PER_MINUTE;
    }
}

void Timer::onTimeout()
{
    if (isStopwatch)
    {
        elapsedTime += TIMER_INTERVAL_MS;

        int currentSecond = elapsedTime / MS_PER_SECOND;
        if (currentSecond > lastSecond)
        {
            if (tickSound)
                tickSound->play();
            lastSecond = currentSecond;
        }

        if (totalTime > 0 && elapsedTime >= totalTime)
        {
            elapsedTime = totalTime;
            emitStopwatchTime(elapsedTime);
            internalTimer->stop();
            if (endSound)
                endSound->play();
            emit finished();
            return;
        }

        emitStopwatchTime(elapsedTime);
    }
    else
    {
        int currentSecond = (remainingTime - 1) / MS_PER_SECOND;
        if (currentSecond < lastSecond && remainingTime > 0)
        {
            if (tickSound)
                tickSound->play();
            lastSecond = currentSecond;
        }

        emitCountdownTime(remainingTime);

        if (remainingTime <= 0)
        {
            internalTimer->stop();
            if (endSound)
                endSound->play();
            emit finished();
        }
        else
            remainingTime -= TIMER_INTERVAL_MS;
    }
}

void Timer::emitCountdownTime(int milliseconds)
{
    int min = milliseconds / MS_PER_MINUTE;
    int sec = (milliseconds % MS_PER_MINUTE) / MS_PER_SECOND;

    QString timeStr = QString("%1:%2").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0'));

    float progress = (totalTime > 0) ? (float)remainingTime / totalTime : 0.0f;
    emit timeUpdated(timeStr, progress);
}

void Timer::emitStopwatchTime(int milliseconds)
{
    int min = milliseconds / MS_PER_MINUTE;
    int sec = (milliseconds % MS_PER_MINUTE) / MS_PER_SECOND;
    int ms = (milliseconds % MS_PER_SECOND) / 10;

    QString timeStr = QString("%1:%2.%3").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(ms, 2, 10, QChar('0'));

    float progress = (totalTime > 0) ? (float)elapsedTime / totalTime : 0.0f;
    emit timeUpdated(timeStr, progress);
}