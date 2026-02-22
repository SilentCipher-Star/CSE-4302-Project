#ifndef TIMER_HPP
#define TIMER_HPP

#include <QObject>
#include <QTimer>

class Timer : public QObject
{
    Q_OBJECT
public:
    explicit Timer(QObject *parent = nullptr);

    void start(int minutes);
    void pause();
    void stop();

signals:
    void timeUpdated(QString time, float progress);
    void finished();

private slots:
    void onTimeout();

private:
    QTimer *internalTimer;
    int remainingTime;
    int totalTime;
    bool isPaused;
};
#endif // TIMER_HPP