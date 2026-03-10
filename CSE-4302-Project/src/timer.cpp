#include "../include/timer.hpp"

Timer::Timer(QObject *parent) : QObject(parent), remainingTime(0), totalTime(0), elapsedTime(0), isPaused(false), isStopwatch(false)
{
    internalTimer = new QTimer(this);
    connect(internalTimer, &QTimer::timeout, this, &Timer::onTimeout);
}

void Timer::start(double minutes)
{
    if (minutes <= 0)
        return;
    remainingTime = static_cast<int>(minutes * 60 * 1000);
    totalTime = remainingTime;
    isPaused = false;
    isStopwatch = false;
    internalTimer->start(50);
    onTimeout();
}

void Timer::startStopwatch(double targetMinutes)
{
    isStopwatch = true;
    elapsedTime = 0;
    totalTime = static_cast<int>(targetMinutes * 60 * 1000); // Used for progress calculation only
    isPaused = false;
    internalTimer->start(50);
    onTimeout();
}

void Timer::pause()
{
    if (internalTimer->isActive())
    {
        internalTimer->stop();
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
        {
            canResume = true;
        }
        if (canResume)
        {
            internalTimer->start(50);
            isPaused = false;
        }
    }
}

void Timer::stop()
{
    internalTimer->stop();
    isPaused = false;
    remainingTime = 0;
    elapsedTime = 0;
    emit timeUpdated("00:00", 0.0f);
}

double Timer::getElapsedMinutes() const
{
    if (isStopwatch)
    {
        return (double)elapsedTime / 60000.0;
    }
    else
    {
        if (totalTime <= 0)
            return 0.0;
        return (double)(totalTime - remainingTime) / 60000.0;
    }
}

void Timer::onTimeout()
{
    if (isStopwatch)
    {
        elapsedTime += 50;

        if (totalTime > 0 && elapsedTime >= totalTime)
        {
            elapsedTime = totalTime;
            int min = elapsedTime / 60000;
            int sec = (elapsedTime % 60000) / 1000;
            int ms = (elapsedTime % 1000) / 10;
            QString timeStr = QString("%1:%2.%3").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(ms, 2, 10, QChar('0'));

            emit timeUpdated(timeStr, 1.0f);
            internalTimer->stop();
            emit finished();
            return;
        }

        int min = elapsedTime / 60000;
        int sec = (elapsedTime % 60000) / 1000;
        int ms = (elapsedTime % 1000) / 10;

        QString timeStr = QString("%1:%2.%3").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0')).arg(ms, 2, 10, QChar('0'));

        float progress = 0.0f;
        if (totalTime > 0)
        {
            progress = (float)elapsedTime / totalTime;
            if (progress > 1.0f)
                progress = 1.0f;
        }
        emit timeUpdated(timeStr, progress);
    }
    else
    {
        int min = remainingTime / 60000;
        int sec = (remainingTime % 60000) / 1000;

        QString timeStr = QString("%1:%2").arg(min, 2, 10, QChar('0')).arg(sec, 2, 10, QChar('0'));

        float progress = (totalTime > 0) ? (float)remainingTime / totalTime : 0.0f;
        emit timeUpdated(timeStr, progress);

        if (remainingTime <= 0)
        {
            internalTimer->stop();
            emit finished();
        }
        else
        {
            remainingTime -= 50;
        }
    }
}